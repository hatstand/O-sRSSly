#!/bin/sh

fixlibs() {
	echo $1
	install_name_tool -id `basename $1` $1
	finklibs=`otool -L $1 | grep '/sw/lib' | grep -v $1 | cut -d ' ' -f 1 | sed -e 's#^ *##'`
	for i in $finklibs
	do
		cp $i $FRMDIR
		finklib=`basename $i`
		echo $finklib
		install_name_tool -change $i @executable_path/../Frameworks/${finklib} $1
		echo $i | grep -q `basename $1` || fixlibs ${FRMDIR}/$finklib
	done
}

NAME="@BINARY_NAME@"

EXEDIR="${NAME}.app/Contents/MacOS"
RESDIR="${NAME}.app/Contents/Resources"
FRMDIR="${NAME}.app/Contents/Frameworks"
PLUGDIR="${NAME}.app/Contents/plugins"

QTDIR="@QT_LIBRARY_DIR@"
QT_PLUGINS="@QT_PLUGINS_DIR@"

mkdir -p $EXEDIR
mkdir $RESDIR
mkdir $FRMDIR
mkdir $PLUGDIR
cp ${NAME} $EXEDIR/${NAME}
cp Info.plist ${NAME}.app/Contents
cp qt.conf.BUNDLEME ${RESDIR}/qt.conf
cp ${NAME}.icns ${RESDIR}

copyqtlib()
{
	cp -R ${QTDIR}/${1}.framework ${FRMDIR}
	rm -rf ${FRMDIR}/${1}.framework/Headers/
	rm -f ${FRMDIR}/${1}.framework/Versions/4/${1}_debug
	install_name_tool -id @executable_path/../Frameworks/${1}.framework/Versions/4.0/${1} ${FRMDIR}/${1}.framework/Versions/4/${1}
}

# Copy in Qt frameworks
copyqtlib QtCore
copyqtlib QtGui
copyqtlib QtSvg
copyqtlib QtXml
copyqtlib QtDBus
copyqtlib QtNetwork
copyqtlib QtWebKit
copyqtlib QtSql
#copyqtlib phonon

# Copy in plugins
cp -R ${QT_PLUGINS}/imageformats $PLUGDIR
rm -f ${PLUGDIR}/imageformats/*_debug.dylib
mkdir ${PLUGDIR}/sqldrivers
cp ${QT_PLUGINS}/sqldrivers/libqsqlite.dylib ${PLUGDIR}/sqldrivers/

# Fix path names to Qt in ${NAME}
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4.0/QtCore $EXEDIR/${NAME}
install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4.0/QtGui $EXEDIR/${NAME}
install_name_tool -change QtNetwork.framework/Versions/4/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/4.0/QtNetwork $EXEDIR/${NAME}
install_name_tool -change QtXml.framework/Versions/4/QtXml @executable_path/../Frameworks/QtXml.framework/Versions/4.0/QtXml $EXEDIR/${NAME}
install_name_tool -change QtSql.framework/Versions/4/QtSql @executable_path/../Frameworks/QtSql.framework/Versions/4.0/QtSql $EXEDIR/${NAME}
install_name_tool -change QtWebKit.framework/Versions/4/QtWebKit @executable_path/../Frameworks/QtWebKit.framework/Versions/4.0/QtWebKit $EXEDIR/${NAME}

# Fix path names in Qt Libraries
# QtCore
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4.0/QtCore $FRMDIR/QtGui.framework/Versions/4.0/QtGui
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4.0/QtCore $FRMDIR/QtSvg.framework/Versions/4.0/QtSvg
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4.0/QtCore $FRMDIR/QtXml.framework/Versions/4.0/QtXml
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4.0/QtCore $FRMDIR/QtNetwork.framework/Versions/4.0/QtNetwork
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4.0/QtCore $FRMDIR/QtDBus.framework/Versions/4.0/QtDBus
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4.0/QtCore $FRMDIR/QtSql.framework/Versions/4.0/QtSql
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4.0/QtCore $FRMDIR/QtWebKit.framework/Versions/4.0/QtWebKit

# QtGui
install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4.0/QtGui $FRMDIR/QtSvg.framework/Versions/4.0/QtSvg

# QtDBus
install_name_tool -change QtXml.framework/Versions/4/QtXml @executable_path/../Frameworks/QtXml.framework/Versions/4.0/QtXml $FRMDIR/QtDBus.framework/Versions/4.0/QtDBus

# QtWebKit
install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4.0/QtGui $FRMDIR/QtWebKit.framework/Versions/4.0/QtWebKit
install_name_tool -change QtNetwork.framework/Versions/4/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/4.0/QtNetwork $FRMDIR/QtWebKit.framework/Versions/4.0/QtWebKit


# Fix plugins
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4.0/QtCore $PLUGDIR/imageformats/libqgif.dylib
install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4.0/QtGui $PLUGDIR/imageformats/libqgif.dylib
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4.0/QtCore $PLUGDIR/imageformats/libqico.dylib
install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4.0/QtGui $PLUGDIR/imageformats/libqico.dylib
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4.0/QtCore $PLUGDIR/imageformats/libqjpeg.dylib
install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4.0/QtGui $PLUGDIR/imageformats/libqjpeg.dylib
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4.0/QtCore $PLUGDIR/imageformats/libqmng.dylib
install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4.0/QtGui $PLUGDIR/imageformats/libqmng.dylib
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4.0/QtCore $PLUGDIR/imageformats/libqsvg.dylib
install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4.0/QtGui $PLUGDIR/imageformats/libqsvg.dylib
install_name_tool -change QtXml.framework/Versions/4/QtXml @executable_path/../Frameworks/QtXml.framework/Versions/4.0/QtXml $PLUGDIR/imageformats/libqsvg.dylib
install_name_tool -change QtSvg.framework/Versions/4/QtSvg @executable_path/../Frameworks/QtSvg.framework/Versions/4.0/QtSvg $PLUGDIR/imageformats/libqsvg.dylib
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4.0/QtCore $PLUGDIR/imageformats/libqtiff.dylib
install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4.0/QtGui $PLUGDIR/imageformats/libqtiff.dylib

install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4.0/QtCore $PLUGDIR/sqldrivers/libqsqlite.dylib
install_name_tool -change QtSql.framework/Versions/4/QtSql @executable_path/../Frameworks/QtSql.framework/Versions/4.0/QtSql $PLUGDIR/sqldrivers/libqsqlite.dylib
# Fix other libraries
fixlibs ${EXEDIR}/${NAME}

