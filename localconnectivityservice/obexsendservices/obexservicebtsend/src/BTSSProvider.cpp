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
#include <btserviceapi.h>
#include <AiwVariantType.hrh>
#include <AiwVariant.h>
#include <AiwMenu.h>
#include <f32file.h>
#include <btfeaturescfg.h>	// For Enterprise security settings
#include <btnotif.h>	// For Enterprise security notifier
#include <data_caging_path_literals.hrh> 
#include <BtSSMenu.rsg>

#include "BTSSProvider.h"
#include "BTSendingServiceDebug.h"
#include "BTSSSendListHandler.h"

_LIT( KBTSendingServiceFileDrive, "z:");
_LIT( KBTSSResFileName,"BtSSMenu.rsc");

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBTSSProvider::CBTSSProvider
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBTSSProvider::CBTSSProvider(): iConverter(NULL)
	{
	}

// -----------------------------------------------------------------------------
// CBTSSProvider::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CBTSSProvider::ConstructL()
    {
    FLOG(_L("[BTSS]\t CBTSSProvider::ConstructL()"));
    }

// -----------------------------------------------------------------------------
// CBTSSProvider::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBTSSProvider* CBTSSProvider::NewL()
    {
    CBTSSProvider* self = new( ELeave ) CBTSSProvider;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// -----------------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------------
//
CBTSSProvider::~CBTSSProvider()
	{
    FLOG(_L("[BTSS]\t CBTSSProvider::destructor"));
	
    if( iBTSendingService )
        {
        delete iBTSendingService;
        }
    
    delete iConverter;
    iConverter = NULL;
	}

// -----------------------------------------------------------------------------
// CBTSSProvider::InitialiseL
// -----------------------------------------------------------------------------
//
void CBTSSProvider::InitialiseL(MAiwNotifyCallback& /*aFrameworkCallback*/,
								      const RCriteriaArray& /*aInterest*/)
	{
	// Not needed.
	}

// -----------------------------------------------------------------------------
// CBTSSProvider::HandleServiceCmdL
// -----------------------------------------------------------------------------
//
void CBTSSProvider::HandleServiceCmdL(const TInt& aCmdId,
								    	    const CAiwGenericParamList& aInParamList,
											CAiwGenericParamList& /*aOutParamList*/,
											TUint /*aCmdOptions*/,
											const MAiwNotifyCallback* aCallback )
	{	
    FLOG(_L("[BTSS]\t CBTSSProvider::HandleServiceCmdL()"));
	
    if (  &aInParamList == NULL  || aInParamList.Count() <= 0 ) 
        {
        FLOG(_L("[BTSS]\t CBTSSProvider::HandleServiceCmdL() aOutParamList check failed: Leave"));
        User::Leave( KErrArgument );
        }
    
    if ( aCallback )
        {
        FLOG(_L("[BTSS]\t CBTSSProvider::HandleServiceCmdL() aCallback exists: Leave"));
        User::Leave( KErrNotSupported );
        }

    switch ( aCmdId )
        {
        case KAiwCmdSend:
            {
			// Check features setting - if not completely enabled with enterprise settings then we are not allowed to send anything.
			// Fail here at the first fence, otherwise there are a number of other areas that need to be considered.
			if(BluetoothFeatures::EnterpriseEnablementL() != BluetoothFeatures::EEnabled)
				{
				RNotifier notifier;
				User::LeaveIfError(notifier.Connect());
				CleanupClosePushL(notifier);
				User::LeaveIfError(notifier.StartNotifier(KBTEnterpriseItSecurityInfoNotifierUid, KNullDesC8));
				CleanupStack::PopAndDestroy(&notifier);
				// Don't leave as we have already commuicated (through the security notifier) why we failed.
				break;
				}
			
            if ( !iBTSendingService )
                {
                // Create the controller when needed
                //
                iBTSendingService = CBTServiceAPI::NewL();
                }            
            CBTServiceParameterList* parameterList = CBTServiceParameterList::NewLC();       
            
            iConverter = CBTSSSendListHandler::NewL();
            User::LeaveIfError(iConverter->ConvertList( &aInParamList, parameterList));
			
                delete iConverter;
                iConverter = NULL;

            // Start sending files. This function returns when all of the files are sent
            // or some error has occured.
            //          
            
			CleanupStack::Pop(parameterList);
            iBTSendingService->StartSynchronousServiceL( EBTSendingService, parameterList ); 
           
			 break;
            }
		default:
            {
            FLOG(_L("[BTSS]\t CBTSSProvider::HandleServiceCmdL() wrong command id: Leave"));
            User::Leave( KErrNotSupported );
            break;
            }
        }

    FLOG(_L("[BTSS]\t CBTSSProvider::HandleServiceCmdL() completed"));
    }

	
 void CBTSSProvider::HandleMenuCmdL(TInt aMenuCmdId, 
                                    const CAiwGenericParamList& aInParamList,
                                    CAiwGenericParamList& aOutParamList,
                                    TUint aCmdOptions,
                                    const MAiwNotifyCallback* aCallback )
    {
    HandleServiceCmdL(aMenuCmdId,aInParamList, aOutParamList, aCmdOptions, aCallback);
    }
    
 void   CBTSSProvider::InitializeMenuPaneL(  CAiwMenuPane& aMenuPane,
                                            TInt aIndex,
                                            TInt /* aCascadeId */,
                                            const CAiwGenericParamList& /*aInParamList*/ )
    {
    TFileName resourceFile;
    TInt resId;
    
    resourceFile += KBTSendingServiceFileDrive;
    resourceFile += KDC_RESOURCE_FILES_DIR;
    resourceFile += KBTSSResFileName;    
    resId=R_SEND_VIA_BT_MENU;
    
    aMenuPane.AddMenuItemsL(
            resourceFile, 
            resId,
            KAiwCmdSend,
            aIndex);
    
    }

// End of file
