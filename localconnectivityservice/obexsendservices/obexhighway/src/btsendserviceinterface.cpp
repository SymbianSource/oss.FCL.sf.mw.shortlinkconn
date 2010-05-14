/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  ?Description
*
*/

 
#include "btsendserviceinterface.h"
#include "btsendserviceprovider.h"

BTSendServiceInterface::BTSendServiceInterface(QObject* parent)
: XQServiceProvider("com.nokia.services.btsendservice.imessage.send",parent)
    {
    publishAll();
    }

  
void BTSendServiceInterface::send(QVariant data)
    {
    QList<QVariant> arguments;
    
    if(data.type()==QVariant::String)
        {
        arguments.append(data);
        }
    else
        {
        arguments.append(data.toList());
        }
    CBtSendServiceProvider *btSendServiceProvider = NULL;
    TRAPD(err,btSendServiceProvider = CBtSendServiceProvider::NewL());
    if(err)
        return;
    btSendServiceProvider->send(arguments);
    delete btSendServiceProvider;
    }
