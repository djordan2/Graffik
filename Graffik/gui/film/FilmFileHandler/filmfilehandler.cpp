/*

  Graffik Motion Control Application

  Copyright (c) 2011-2012 Dynamic Perception

 This file is part of Graffik.

    Graffik is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Graffik is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Graffik.  If not, see <http://www.gnu.org/licenses/>.

    */

#include "filmfilehandler.h"

FilmFileHandler::FilmFileHandler() {

}

FilmFileHandler::~FilmFileHandler() {

}

/** Static Write File

  Writes a given set of FilmParameters to a file.  Persists a copy of OMfilmParams as returned
  by the FilmParameters object using the FilmParameters::getParamsCopy() method.

  @param p_file
  File to write to

  @param p_params
  A FilmParameters object pointer containing the parameters to persist to disk

  @param p_showErr
  Whether or not to report errors back to user (default = true)

*/
void FilmFileHandler::writeFile(QString p_file, FilmParameters* p_params, bool p_showErr) {

        // do not try to write to a file with no name
    if( p_file.isEmpty() )
        return;


    QFile filmFile(p_file);
    QString errors;
    bool errorOccur = false;


    if( filmFile.open(QIODevice::WriteOnly) ) {
        QDataStream data( &filmFile );
            // write structure version to file
        data << quint32(OM_FILM_FMT_VER);
            // write film data structure to file
        data << p_params->getParamsCopy();
        filmFile.close();
    }
    else {
        errors.append("ERROR: Could not open file ");
        errors.append(p_file);
        errors.append(": ");
        errors.append( filmFile.error() );
        errorOccur = true;
    }

    if( p_showErr && errorOccur ) {
        ErrorDialog erDlg;
        erDlg.setError(errors);
        erDlg.exec();
    }

}

/** Static Read File


  Restores a persisted OMfilmParams structure from a file into a given FilmParameters object.

  @param p_file
  The path to the file to open

  @param p_params
  A pointer to a valid FilmParameters object

  @param p_net
  A pointer to a valid OMNetwork object (required for validating devices loaded)

  @param p_showErr
  Whether or not to show loading errors via an ErrorDialog after loading the file.  Defaults to true

  */

void FilmFileHandler::readFile(QString p_file, FilmParameters* p_params, OMNetwork* p_net, bool p_showErr) {

    if( p_file.isEmpty() )
        return;

    QFile filmFile(p_file);
    QString errors;
    bool errorOccur = false;


    if( filmFile.open(QIODevice::ReadOnly) ) {
        QDataStream data( &filmFile );
        OMfilmParams newParams;
        qint32 fileVersion;

        data >> fileVersion;

            // check version written to file
        if( fileVersion != OM_FILM_FMT_VER ) {
            errors.append("ERROR: Incorrect File Version, Cannot Load\n");
            errorOccur = true;
        }
        else {

                // stream in data from file
            data >> newParams;

            OMfilmParams fp = p_params->getParamsCopy();
            OMfilmParams* nparm = new OMfilmParams(newParams);



                // validate that all devices loaded from file are known
                // and do any required re-mapping

            _validateDevices(&fp, nparm, p_net);

                // TODO: Fix memory leak from OMfilmCamParams and list of OMfilmAxisParams

            OMfilmParams* ptr = p_params->getParams();
            *ptr = *nparm;
            p_params->releaseParams();
        }

        filmFile.close();

    }
    else {
        errors.append("ERROR: Could not open file ");
        errors.append(p_file);
        errors.append(": ");
        errors.append( filmFile.error() );
        errorOccur = true;
    }

    if( p_showErr && errorOccur ) {
        ErrorDialog erDlg;
        erDlg.setError(errors);
        erDlg.exec();
    }
}


void FilmFileHandler::_validateDevices(OMfilmParams* p_current, OMfilmParams* p_new, OMNetwork* p_net) {

    QHash<unsigned short, OMdeviceInfo*> knownDevs = p_net->getDevices();
    QList<unsigned short> notFound;
    QHash<unsigned short, QString> available;

        // search for unknown devices that were loaded in the file
    foreach(unsigned short addr, p_new->axes.keys() ) {
        if( ! knownDevs.contains(addr) ) {
            qDebug() << "FFH: Validate Devices: Could not find device address" << addr;
            notFound.append(addr);
        }
    }

        // locate all devices that exist, but weren't in the film

    foreach(unsigned short addr, p_current->axes.keys() ) {
        if( ! p_new->axes.contains(addr) ) {
            qDebug() << "FFH: Validate Devices: Device was not in file" << addr;
            QString devName = knownDevs.value(addr)->name;
            available.insert(addr, devName);
        }
    }

        // no addresses available to use... Throw out invalid devices
    if( notFound.count() > 0 && available.count() == 0 ) {
        qDebug() << "FFH: Validate Devices: Devices Remain with No Available Targets";
        foreach(unsigned short addr, notFound)
            p_new->axes.remove(addr);
    }
    else if(notFound.count() > 0 && available.count() > 0 ) {
        // if we have anything that wasn't found, and we have addresses available, then
        // see if the user wants to remap each one


        foreach(unsigned short addr, notFound) {

            AddressNotFoundDialog mapdia(addr, available);
            int ret = mapdia.exec();

            qDebug() << "FFH: Got Return Code" << ret;

            if( ret == 0 || ret == 1 ) {
                qDebug() << "FFH: Validate Devices: User Chose Not to Remap" << addr;

                // they didn't choose a valid device... remove the found one
                p_new->axes.remove(addr);
            }
            else {
                    // copy to new location
                qDebug() << "FFH: Validate Devices: User Chose to Remap" << addr << "to" << ret;
                OMfilmAxisParams* axparms = p_new->axes.value(addr);
                p_new->axes.insert(ret, new OMfilmAxisParams(*axparms));
                p_new->axes.remove(addr);
                available.remove(ret);
            }
        }

            // wipe out list
        notFound.clear();

    }

    if( notFound.count() == 0 && available.count() > 0 ) {
        // dang, we have devices not found in the file - make sure we keep those guys
        // and don't lose their OMfilmAxisParameters (or its crash city for us!)
        foreach( unsigned short addr, available.keys() ) {
            qDebug() << "FHH: Adding Back Address not in Film File" << addr;
            p_new->axes.insert(addr, p_current->axes.value(addr));
        }
    }

}
