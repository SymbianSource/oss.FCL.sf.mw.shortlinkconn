/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Converter class implementation for converting AIW paramerer 
*                list to bt send parameter list
*
*/


#include <AiwGenericParam.h>

#include "BTSSSendListHandler.h"
#include "btserviceparameterlist.h"
#include "BTSendingServiceDebug.h"



// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CBTSSSendListHandler()
// ---------------------------------------------------------------------------
//
CBTSSSendListHandler::CBTSSSendListHandler(): CActive( EPriorityNormal )
    {
    CActiveScheduler::Add(this);
    }

// ---------------------------------------------------------------------------
// ConstructL()
// ---------------------------------------------------------------------------
//
void CBTSSSendListHandler::ConstructL()
    {
    }


// ---------------------------------------------------------------------------
// NewL()
// ---------------------------------------------------------------------------
//
CBTSSSendListHandler* CBTSSSendListHandler::NewL()
    {
    CBTSSSendListHandler* self = CBTSSSendListHandler::NewLC();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// NewLC()
// ---------------------------------------------------------------------------
//
CBTSSSendListHandler* CBTSSSendListHandler::NewLC()
    {
    CBTSSSendListHandler* self = new( ELeave ) CBTSSSendListHandler();
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }


// ---------------------------------------------------------------------------
// CBTSSSendListHandler
// ---------------------------------------------------------------------------
//
CBTSSSendListHandler::~CBTSSSendListHandler()    
    {
    FLOG(_L("[BTSS]\t CBTSSSendListHandler::Destructor"));     
    Cancel();
    }

// ---------------------------------------------------------------------------
// ConvertList()
// ---------------------------------------------------------------------------
//
TInt CBTSSSendListHandler::ConvertList( const CAiwGenericParamList* aOutParamList, 
                                        CBTServiceParameterList* aList)
    {
    FLOG(_L("[BTSS]\t CBTSSSendListHandler::ConvertList"));           
    iListIndex = 0;    
    iOutParamList = aOutParamList;
    iList = aList;
    iStatus = KRequestPending;
    AddObject();
    SetActive();
    
    iSyncWaiter.Start();
    FLOG(_L("[BTSS]\t CBTSSSendListHandler::ConvertList done"));           
    return iStatus.Int();
    }

// ---------------------------------------------------------------------------
// AddObject()
// ---------------------------------------------------------------------------
//
void CBTSSSendListHandler::AddObject()
    {
    FLOG(_L("[BTSS]\t CBTSSSendListHandler::AddObjectL()"));       
    TRAPD(error, DoAddObjectL());
    if (error != KErrNone )
        {        
        //Complete request
        TRequestStatus* status = &iStatus;
        User::RequestComplete(status, error);
        }
    FLOG(_L("[BTSS]\t CBTSSSendListHandler::AddObjectL() Done"));           
    }

// ---------------------------------------------------------------------------
// From class CActive.
// RunL()
// ---------------------------------------------------------------------------
//
void CBTSSSendListHandler::DoAddObjectL()
    {
    FLOG(_L("[BTSS]\t CBTSSSendListHandler::DoAddObjectL()"));   
    
    const TAiwGenericParam* param=&(*iOutParamList)[iListIndex];

    if ( param->SemanticId() != EGenericParamFile )
        {
        FLOG(_L("[BTSS]\t CBTSSSendListHandler::DoAddObjectL() wrong semantic Id: Leave"));            
        }
      // Try to add file as an image
      //       
    if( param->Value().TypeId()== EVariantTypeFileHandle)
        {           
        TRAPD( retVal, iList->AddImageL( param->Value().AsFileHandle()  ));
        FLOG(_L("[BTSS]\t CBTSSSendListHandler::DoAddObjectL AddImage"));            
        if( retVal != KErrNone )
            {
            // File was not an image. We can not use BIP 
            // so add rest of the files as objects
            //             
            FLOG(_L("[BTSS]\t CBTSSSendListHandler::DoAddObjectL AddImage filed try object"));            
            iList->AddObjectL( param->Value().AsFileHandle() );
            }            
        }
    else 
        {           
        TRAPD( retVal, iList->AddImageL( param->Value().AsDes() ) );
        if( retVal != KErrNone )
            {
            // File was not an image. We can not use BIP 
            // so add rest of the files as objects
            //                
            iList->AddObjectL( param->Value().AsDes() );
            }
        }
      //Complete request
    TRequestStatus* status = &iStatus;
    User::RequestComplete(status, KErrNone);
      
    FLOG(_L("[BTSS]\t CBTSSSendListHandler::DoAddObjectL() completed"));  
    }
// ---------------------------------------------------------------------------
// From class CActive.
// RunL()
// ---------------------------------------------------------------------------
//
void CBTSSSendListHandler::RunL()
    {
    FLOG(_L("[BTSS]\t CBTSSSendListHandler::DoAddObjectL()"));       
    iListIndex++;
    if ( iOutParamList->Count() > iListIndex && iStatus.Int() == KErrNone )
        {        
        iStatus = KRequestPending;
        AddObject();
        SetActive();
        }
    else
        {
        if ( iSyncWaiter.IsStarted() )
            {
            iSyncWaiter.AsyncStop();
            }
        }
    FLOG(_L("[BTSS]\t CBTSSSendListHandler::DoAddObjectL() Done"));           
    }
// ---------------------------------------------------------------------------
// From class CActive.
// DoCancel()
// ---------------------------------------------------------------------------
//
void CBTSSSendListHandler::DoCancel()
    {
    FLOG(_L("[BTSS]\t CBTSSSendListHandler::DoCancel()"));           
    if ( iSyncWaiter.IsStarted() )
        {
        iSyncWaiter.AsyncStop();
        }
    FLOG(_L("[BTSS]\t CBTSSSendListHandler::DoCancel() done"));               
    }
