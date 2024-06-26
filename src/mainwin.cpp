/*-
 * Copyright (c) 2019 The NomadBSD Project. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <QCompleter>
#include <QFormLayout>
#include <QProcess>
#include <QDebug>

#include "defs.h"
#include "mainwin.h"
#include "qt-helper/qt-helper.h"

#define PREVIEW_WIDTH	512
#define PREVIEW_HEIGHT	384

MainWin::MainWin(QWidget *parent) : QMainWindow(parent) {
	themeCbB	     = new QComboBox;
	usernameCbB	     = new QComboBox;
	sessionCbB	     = new QComboBox;
	previewImage	     = new QLabel;
	autologinCB	     = new QCheckBox(tr("Auto login"));
	defaultUserCB	     = new QCheckBox(tr("Set default user"));
	defaultUserContainer = new QWidget;

	querySettings();
	autologinCB->setTristate(false);
	defaultUserCB->setTristate(false);
	setWindowTitle(tr("Display manager settings"));
	setWindowIcon(qh_loadIcon("preferences-desktop-personal", NULL));
	statusBar()->showMessage("");

	QVBoxLayout *layout = new QVBoxLayout;
	layout->setContentsMargins(15, 15, 15, 15);

	QFormLayout *themeForm  = new QFormLayout;
	QLabel	    *themeLabel = new QLabel(tr("Theme"));
	themeForm->addRow(themeLabel, themeCbB);
	layout->addLayout(themeForm);

	QLabel *previewLabel = new QLabel(tr("Preview"));
	previewLabel->setStyleSheet("font-weight: bold;");
	layout->addWidget(previewLabel);
	layout->addWidget(previewImage, 1, Qt::AlignCenter);
	layout->addStretch(1);
	layout->addWidget(mkLine());

	if (cfg_dm == "slim")
		layout->addWidget(defaultUserCB);

	QFormLayout *form	   = new QFormLayout;
	QLabel	    *userLabel = new QLabel(tr("Default user"));
	form->addRow(userLabel, usernameCbB);

	QLabel *sessionLabel = new QLabel(tr("Default Session"));
	form->addRow(sessionLabel, sessionCbB);

	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addLayout(form);
	vbox->addWidget(autologinCB);
	if (cfg_dm == "slim") {
		defaultUserContainer->setLayout(vbox);
		layout->addWidget(defaultUserContainer);
	} else
		layout->addLayout(vbox);
	layout->addWidget(mkLine());

	QIcon quitIcon = qh_loadStockIcon(QStyle::SP_DialogCloseButton);
	QIcon saveIcon = qh_loadIcon("document-save", NULL);
	if (saveIcon.isNull())
		saveIcon = qh_loadStockIcon(QStyle::SP_DialogSaveButton);
	QPushButton *save = new QPushButton(saveIcon, tr("&Save"), this);
	QPushButton *quit = new QPushButton(quitIcon, tr("&Quit"), this);

	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->addWidget(save,  1, Qt::AlignRight);
	hbox->addWidget(quit,  0, Qt::AlignRight);
	layout->addLayout(hbox);

	QWidget *container = new QWidget;
	container->setLayout(layout);
	setCentralWidget(container);

	init();

	connect(themeCbB, SIGNAL(currentIndexChanged(int)), this,
	    SLOT(themeChanged(int))); 
	connect(usernameCbB, SIGNAL(currentIndexChanged(int)), this,
	    SLOT(usernameChanged(int)));
	if (cfg_dm == "sddm") {
		connect(sessionCbB, SIGNAL(currentIndexChanged(int)), this,
			SLOT(sessionChanged(int)));
	}
	connect(autologinCB, SIGNAL(stateChanged(int)), this,
	    SLOT(autologinCBClicked(int)));
	if (cfg_dm == "slim") {
		connect(defaultUserCB, SIGNAL(stateChanged(int)), this,
			SLOT(defaultUserCBClicked(int)));
	}
	connect(save, SIGNAL(clicked(bool)), this, SLOT(save()));
	connect(quit, SIGNAL(clicked(bool)), this, SLOT(quit()));
}

QFrame *MainWin::mkLine(void)
{
	QFrame *line = new QFrame(this);
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);
	return (line);
}

void
MainWin::quit()
{
	QMessageBox msgBox(this);

	if (saved_theme	    == cfg_theme     &&
	    saved_username  == cfg_username  &&
	    saved_autologin == cfg_autologin &&
	    saved_session   == cfg_session)
		std::exit(0);
	msgBox.setWindowModality(Qt::WindowModal);
	msgBox.setText(tr("The file has been modified."));
	msgBox.setWindowTitle(tr("The file has been modified."));
	msgBox.setInformativeText(tr("Do you want to save your changes?"));
	msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard |
	    QMessageBox::Cancel);

	msgBox.setButtonText(QMessageBox::Save, tr("&Save"));
	msgBox.setButtonText(QMessageBox::Discard,
	    tr("&Quit without saving"));
	msgBox.setButtonText(QMessageBox::Cancel, tr("&Cancel"));
	msgBox.setDefaultButton(QMessageBox::Save);
	msgBox.setIcon(QMessageBox::Question);
	QIcon icon = qh_loadStockIcon(QStyle::SP_MessageBoxQuestion);
	msgBox.setWindowIcon(icon);

	switch (msgBox.exec()) {
	case QMessageBox::Save:
		save();
	case QMessageBox::Discard:
		std::exit(0);
	}
}

void
MainWin::closeEvent(QCloseEvent *event)
{
	MainWin::quit();
	event->ignore();
}

void
MainWin::autologinCBClicked(int state)
{
	cfg_autologin = (state == Qt::Unchecked ? "no" : "yes");
	if (cfg_dm == "sddm") {
		usernameCbB->setDisabled(state != Qt::Checked);
		sessionCbB->setDisabled(state != Qt::Checked);
	}
}

void
MainWin::defaultUserCBClicked(int state)
{
	if (state == Qt::Unchecked) {
		autologinCB->setCheckState(Qt::Unchecked);
		defaultUserContainer->setDisabled(true);
		cfg_username = "";
	} else {
		defaultUserContainer->setDisabled(false);
		cfg_username = usernameCbB->currentText();
	}
}

void
MainWin::usernameChanged(int /* unused */)
{
	cfg_username = usernameCbB->currentText();
}

void
MainWin::showPreviewImage(QString &theme)
{
	QString path;

	if (cfg_dm == "sddm") {
		path = QString("%1/%2/%3.jpg").arg(PATH_SDDM_THEME_DIR)
					      .arg(theme).arg(theme);
	} else {
		path = QString("%1/%2/preview.png").arg(PATH_SLIM_THEME_DIR)
						   .arg(theme);
	}
	QPixmap image(path);
	if (image.isNull()) {
		QPixmap placeholder(PREVIEW_WIDTH, PREVIEW_HEIGHT);
		placeholder.fill();
		previewImage->setPixmap(placeholder);
	} else {
		previewImage->setPixmap(image.scaled(PREVIEW_WIDTH,
		    PREVIEW_HEIGHT,
		    Qt::KeepAspectRatio, Qt::FastTransformation));
	}
}

void
MainWin::themeChanged(int /* unused */)
{
	cfg_theme = themeCbB->currentText();
	showPreviewImage(cfg_theme);
}

void
MainWin::sessionChanged(int /* unused */)
{
	cfg_session = sessionCbB->currentText();
}

void
MainWin::save()
{
	QProcess    proc;
	QString	    errstr;
	QByteArray  line;
	QStringList args;

	args << "-d" << cfg_username
	     << "-a" << cfg_autologin
	     << "-t" << cfg_theme
	     << "-s" << cfg_session;

	proc.setReadChannel(QProcess::StandardError);
	proc.start(PATH_BACKEND, args);
	(void)proc.waitForStarted(-1);

	if (proc.state() == QProcess::NotRunning) {
		errAndOut(tr("Couldn't start backend '%1': %2")
		    .arg(PATH_BACKEND).arg(proc.errorString()));
	}
	while (proc.waitForReadyRead(-1)) {
		while (!(line = proc.readLine()).isEmpty())
			errstr.append(line);
        }
	proc.waitForFinished(-1);
	if (proc.exitCode() != 0) {
		errAndOut(tr("Command '%1' returned with an error:\n%2")
		    .arg(PATH_BACKEND).arg(errstr));
	}
	saved_username	= cfg_username;
	saved_theme	= cfg_theme;
	saved_autologin	= cfg_autologin;
	saved_session	= cfg_session;
	statusBar()->showMessage(tr("Saved"), 5000);
}

void
MainWin::init()
{
	int idx;

	autologinCB->setCheckState(cfg_autologin == "no" ? \
	    Qt::Unchecked : Qt::Checked);
	if (cfg_username.length() > 0) {
		defaultUserContainer->setDisabled(false);
		defaultUserCB->setCheckState(Qt::Checked);
	} else {
		defaultUserContainer->setDisabled(true);
		defaultUserCB->setCheckState(Qt::Unchecked);
	}
	sessionCbB->addItems(sessions);
	themeCbB->addItems(themes);
	usernameCbB->addItems(usernames);
	idx = themeCbB->findText(cfg_theme, Qt::MatchExactly);
	if (idx >= 0)
		themeCbB->setCurrentIndex(idx);
	idx = sessionCbB->findText(cfg_session, Qt::MatchExactly);
	if (idx >= 0)
		sessionCbB->setCurrentIndex(idx);
	idx = usernameCbB->findText(cfg_username, Qt::MatchExactly);
	if (idx >= 0)
		usernameCbB->setCurrentIndex(idx);
	if (cfg_username == "")
		cfg_username = usernameCbB->currentText();
	if (cfg_dm == "sddm") {
		usernameCbB->setDisabled(autologinCB->checkState() == Qt::Unchecked);
		sessionCbB->setDisabled(autologinCB->checkState() == Qt::Unchecked);
	}
	showPreviewImage(cfg_theme);
	saved_username	= cfg_username;
	saved_theme	= cfg_theme;
	saved_autologin	= cfg_autologin;
	saved_session	= cfg_session;
}

void
MainWin::querySettings()
{
	QProcess    proc;
	QByteArray  line;

  	proc.setReadChannel(QProcess::StandardOutput);
	proc.startCommand(PATH_BACKEND" -q");
	(void)proc.waitForStarted(-1);

	//
	// According to the Qt docs, we can not rely on the return value.
	// We have to check state().
	//
	if (proc.state() == QProcess::NotRunning) {
		errAndOut(tr("Couldn't start backend '%1': %2")
		    .arg(PATH_BACKEND).arg(proc.errorString()));
	}
	while (proc.waitForReadyRead(-1)) {
		while (!(line = proc.readLine()).isEmpty()) {
			// Remove trailing newline
			line.truncate(line.size() - 1);
			if (line.startsWith("themes=")) {
				line.remove(0, 7);
				QList<QByteArray>list = line.split(' ');
				for (int i = 0; i < list.count(); i++)
					themes.append(QString(list.at(i)));
			} else if (line.startsWith("sessions=")) {
				line.remove(0, 9);
				QList<QByteArray>list = line.split(' ');
				for (int i = 0; i < list.count(); i++)
					sessions.append(QString(list.at(i)));
			} else if (line.startsWith("usernames=")) {
				line.remove(0, 10);
				QList<QByteArray>list = line.split(' ');
				for (int i = 0; i < list.count(); i++)
					usernames.append(QString(list.at(i)));
			} else if (line.startsWith("theme=")) {
				line.remove(0, 6);
				cfg_theme = QString(line);
			} else if (line.startsWith("session=")) {
				line.remove(0, 8);
				cfg_session = QString(line);
			} else if (line.startsWith("default_user=")) {
				line.remove(0, 13);
				cfg_username = QString(line);
			} else if (line.startsWith("autologin=")) {
				line.remove(0, 10);
				cfg_autologin = QString(line);
			} else if (line.startsWith("dm=")) {
				line.remove(0, 3);
				cfg_dm = QString(line);
			} else
				continue;
		}
        }
	proc.waitForFinished(-1);
	if (proc.exitCode() != 0) {
		errAndOut(tr("Command '%1' returned with an error.")
		    .arg(PATH_BACKEND));
	}
}
