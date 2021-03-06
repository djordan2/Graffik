/*

  Graffik Motion Control Application

  Copyright (c) 2011-2013 Dynamic Perception

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

#ifndef GLOBALOPTIONS_H
#define GLOBALOPTIONS_H

#include <QObject>
#include <QMutex>


#include "core/Themer/themer.h"
#include "core/Utilities/singleton.h"
#include "optiontypes.h"


/** Global Options Class

  This class manages a set of global options available to the user interface.

  All set and get methods are thread-safe.

  @author
  C. A. Church
  */

class GlobalOptions : public QObject {
    Q_OBJECT
public:
    explicit GlobalOptions(QObject *parent = 0);
    ~GlobalOptions();

    bool stopOnErr();
    void stopOnErr(bool p_mode);

    int display();
    void display(int p_mode);

    QString theme();
    void theme(QString p_theme);

    QHash<int, QString> hotkeys();
    void hotkeys(QHash<int, QString> p_list);

    QString hotkey(int p_key);
    void hotkey(int p_key, QString p_val);
    
signals:
    void optionsChanged();

public slots:
    
private:
    bool m_stopErr;
    int m_display;
    QString m_theme;

    QMutex* m_mutex;

    QHash<int, QString> m_hotKeys;
};

#endif // GLOBALOPTIONS_H
