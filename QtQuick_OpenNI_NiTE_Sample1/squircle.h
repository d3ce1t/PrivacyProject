/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef SQUIRCLE_H
#define SQUIRCLE_H

#include <QtQuick/QQuickItem>
#include <QtGui/QOpenGLShaderProgram>
#include <NiTE.h>
#include <OpenNI.h>
#include "simpleshaderprogram.h"
#include <QElapsedTimer>
#include <QQuickWindow>

class DepthStreamScene;

class Squircle : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(qreal t READ t WRITE setT NOTIFY tChanged)

public:
    Squircle();
    virtual ~Squircle();
    qreal t() const { return m_t; }
    void setT(qreal t);
    void initialize();

signals:
    void tChanged();

protected:
    void itemChange(ItemChange change, const ItemChangeData &);

public slots:
    void paint();
    void cleanup();
    void sync();
    void resizeHeight(int height);
    void resizeWidth(int width);

private:
    void initOpenNI();
    void printMessage(const nite::UserData& user, uint64_t ts, const char *msg);
    void updateUserState(const nite::UserData& user, uint64_t ts);
    void DrawDepthFrame();
    void DrawStatusLabel(nite::UserTracker* pUserTracker, const nite::UserData& user);
    void DrawFrameId(int frameId);
    void DrawCenterOfMass(nite::UserTracker* pUserTracker, const nite::UserData& user, const openni::VideoMode &videoMode);
    void DrawBoundingBox(const nite::UserData& user, const openni::VideoMode &videoMode);
    void DrawLimb(nite::UserTracker* pUserTracker, const nite::SkeletonJoint& joint1, const nite::SkeletonJoint& joint2, int color, const openni::VideoMode &videoMode);
    void DrawSkeleton(nite::UserTracker* pUserTracker, const nite::UserData& userData, const openni::VideoMode &videoMode);


    openni::Device			m_device;
    nite::UserTracker*      m_pUserTracker;
    DepthStreamScene*       m_depthScene;
    openni::VideoMode       videoMode;


    // time to hold in pose to exit program. In milliseconds.
    const int               g_poseTimeoutToExit;
    float static            Colors[4][3];
    int static              colorCount;

    bool                    g_drawSkeleton;
    bool                    g_drawCenterOfMass;
    bool                    g_drawStatusLabel;
    bool                    g_drawBoundingBox;
    bool                    g_drawFrameId;


    SimpleShaderProgram*    m_program2;
    QMatrix4x4              matrix;
    nite::UserId            m_poseUser;
    uint64_t                m_poseTime;
    qreal                   m_t;
    qreal                   m_thread_t;
    QQuickWindow*           win;
};

#endif // SQUIRCLE_H
