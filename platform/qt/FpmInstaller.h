/*!
 * \file FpmInstaller.h
 * \author masc4ii
 * \copyright 2019
 * \brief Install .fpm files to application directory
 */

#ifndef FPMINSTALLER_H
#define FPMINSTALLER_H

#include <QString>
#include <QStringList>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <QStandardPaths>

class FpmInstaller
{
public:
    static bool installFpm( QString fileName )
    {
        //Where to install it?
#ifdef Q_OS_ANDROID
        QString newFileName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).append( "/" ).append( QFileInfo(fileName).fileName() );
#else
        QString newFileName = QCoreApplication::applicationDirPath().append( "/" ).append( QFileInfo(fileName).fileName() );
#endif
        //Remove existing script
        if( QFileInfo( newFileName ).exists() )
        {
            QFile( newFileName ).remove();
        }
        //Copy new one to app
        bool ret = QFile::copy( fileName, newFileName );
        return ret;
    }
    static void installFpm( QStringList *fileNameList )
    {
        for( int i = 0; i < fileNameList->count(); i++ )
        {
            QString fileName = fileNameList->at(i);
#ifdef Q_OS_WIN //Qt Bug?
            if( fileName.startsWith( "/" ) ) fileName.remove( 0, 1 );
#endif
            //Where to install it?
#ifdef Q_OS_ANDROID
            QString newFileName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).append( "/" ).append( QFileInfo(fileName).fileName() );
#else
            QString newFileName = QCoreApplication::applicationDirPath().append( "/" ).append( QFileInfo(fileName).fileName() );
#endif
            //Remove existing script
            if( QFileInfo( newFileName ).exists() )
            {
                QFile( newFileName ).remove();
            }
            //Only install fpm files
            if( !newFileName.endsWith( ".fpm" ) )
            {
                fileNameList->removeAt(i);
                continue;
            }
            //Copy new one to app
            bool ret = QFile::copy( fileName, newFileName );
            if( !ret ) fileNameList->removeAt(i);
        }
        return;
    }
};

#endif // FPMINSTALLER_H
