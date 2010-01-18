/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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


// INCLUDE FILES

#include <AiwCommon.hrh>

#include <AiwMenu.h>
#include <IRSSMenu.rsg>
#include <btnotif.h>        	// Notifier UID's
#include <aknnotewrappers.h> 	//For notifier
#include <featmgr.h>
#include "IrSSProvider.h"
#include "IrSendingServiceDebug.h"
#include "IRClient.h"

#include <data_caging_path_literals.hrh>

_LIT( KIRSendingServiceFileDrive, "z:");
_LIT( KIRSSResFileName,"IRSSMenu.rsc");

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CIRSSProvider::CIRSSProvider
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CIRSSProvider::CIRSSProvider()
	{
	}

// -----------------------------------------------------------------------------
// CIRSSProvider::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CIRSSProvider::ConstructL()
    {
   
    }

// -----------------------------------------------------------------------------
// CIRSSProvider::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CIRSSProvider* CIRSSProvider::NewL()
    {
    CIRSSProvider* self = new( ELeave ) CIRSSProvider;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// -----------------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------------
//
CIRSSProvider::~CIRSSProvider()
	{ 
	
	}

// -----------------------------------------------------------------------------
// CIRSSProvider::InitialiseL
// -----------------------------------------------------------------------------
//
void CIRSSProvider::InitialiseL(MAiwNotifyCallback& /*aFrameworkCallback*/,
								      const RCriteriaArray& /*aInterest*/)
	{
	// Not needed.
	}

// -----------------------------------------------------------------------------
// CIRSSProvider::HandleMenuCmdL
// -----------------------------------------------------------------------------
//

void CIRSSProvider::HandleMenuCmdL( TInt    aCmdId,
								    	    const CAiwGenericParamList& aInParamList,
											CAiwGenericParamList& /*aOutParamList*/,
											TUint /*aCmdOptions*/,
											const MAiwNotifyCallback* /*aCallback*/ )
	{	
    FeatureManager::InitializeLibL();
    if(!FeatureManager::FeatureSupported(KFeatureIdIrda))
        {
   		FLOG(_L("[IRSS]\t FeatMgr doesn't find IrDA, show not_supported "));
	    RNotifier notifier;    
	    User::LeaveIfError( notifier.Connect() );
		TBTGenericInfoNotiferParamsPckg paramsPckg;
		paramsPckg().iMessageType=EIRNotSupported;		
		TInt status = notifier.StartNotifier(KBTGenericInfoNotifierUid, paramsPckg);
	    if ( status != KErrNone )
	        {
	        FTRACE(FPrint(_L("[IRSS]\t void CIRSSProvider::HandleMenuCmdL()  ERROR: StartNotifier() failed. Code: %d "), status));
	        }	    
	    notifier.Close();    	
	    User::Leave(KErrNone);
    	}    	
	FeatureManager::UnInitializeLib();
	
    if ( &aInParamList == NULL || aInParamList.Count() <= 0 )
        {
        FLOG(_L("[IRSS]\t CIRSSProvider::HandleServiceCmdL() aOutParamList check failed: Leave"));
        User::Leave( KErrArgument );
        }

    if ( !iIRClient )
	    {
		iIRClient = CIRClient::NewL();
		}
	iIRClient->StartSendL( aInParamList );

    FLOG(_L("[BTSS]\t CBTSSProvider::HandleServiceCmdL() completed"));
    }
    
 // -----------------------------------------------------------------------------
// CIRSSProvider::InitializeMenuPaneL
// -----------------------------------------------------------------------------
//
    
 void CIRSSProvider::InitializeMenuPaneL(  CAiwMenuPane& aMenuPane,
                                            TInt aIndex,
                                            TInt /* aCascadeId */,
                                            const CAiwGenericParamList& /*aInParamList*/ )
    {
    TFileName resourceFile;
    TInt resId;
    
    resourceFile += KIRSendingServiceFileDrive;
    resourceFile += KDC_RESOURCE_FILES_DIR;
    resourceFile += KIRSSResFileName;    
    resId=R_SEND_VIA_IR_MENU;
    
    aMenuPane.AddMenuItemsL(
            resourceFile, 
            resId,
            KAiwCmdSend,
            aIndex);
    }
    
 void CIRSSProvider::HandleServiceCmdL( const TInt& aCmdId, 
                                        const CAiwGenericParamList& aInParamList,
                                        CAiwGenericParamList& aOutParamList,
                                        TUint aCmdOptions,
                                        const MAiwNotifyCallback* aCallback )
    {
    HandleMenuCmdL(aCmdId,aInParamList,aOutParamList,aCmdOptions,aCallback);
    }

// End of file
