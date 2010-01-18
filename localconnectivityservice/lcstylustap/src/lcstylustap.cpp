/*
* Copyright (c) 2007-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  ECOM  implementation for touch indicator
*
*/


#include <apgtask.h>
#include <ConeResLoader.h>
#include <eikspane.h>
#include <avkon.hrh>
#include <lcstylustaprsc.rsg>
#include <e32uid.h> // KExecutableImageUid
#include <data_caging_path_literals.hrh>
#include "lcstylustap.h"
#include "debug.h"
#include "lcstylustapdismount.h"

const TInt KUsbUIUID = 0x102068E2;
const TInt KBtUIUID  = 0x10005951;


// Constants
_LIT(KFileDrive, "z:");
_LIT( KLCStylustapPluginResourceFile, "lcstylustaprsc.rsc" );
_LIT(KBTUIExe, "BTUI.exe"); // Hard coded name can be used, since it will not be changed
_LIT(KUSBExe, "USBClassChangeUI.exe");


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CLcStylusTap::CLcStylusTap()
    {
    
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//	
void CLcStylusTap::ConstructL()
    {

    }


// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CLcStylusTap* CLcStylusTap::NewL( )
    {
    TRACE_FUNC_ENTRY
    CLcStylusTap* self = new( ELeave ) CLcStylusTap(  );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    TRACE_FUNC_EXIT
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//	    
CLcStylusTap::~CLcStylusTap()
    {
    TRACE_FUNC_ENTRY    
    delete iDismountManager;
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// TextL.
// Return the text that should be displayed in the link.
// ---------------------------------------------------------------------------
//  

HBufC* CLcStylusTap::TextL( const TInt aUid, TInt& aTextType )
    {
        TRACE_INFO((_L("CLcStylusTap::TextL  aUid =  %d"), aUid))
        
 		CEikonEnv* eikEnv = CEikonEnv::Static();
		RConeResourceLoader rLoader(*eikEnv);
		
        TFileName filename;
        filename += KFileDrive;
        filename += KDC_RESOURCE_FILES_DIR; 
        filename += KLCStylustapPluginResourceFile;


        CleanupClosePushL(rLoader);				
		rLoader.OpenL(filename);
		
        HBufC* dynStringBuf = NULL;
        
        switch(aUid)
            {
            case EAknIndicatorBluetooth:
            case EAknIndicatorBluetoothVisible:
            case EAknIndicatorBluetoothModuleOn:
            case EAknIndicatorBluetoothModuleOnVisible:
                {
			    dynStringBuf = eikEnv->AllocReadResourceL(R_LCSTYLUSTAP_BT_NAME );                    
        	    aTextType = EAknIndicatorPluginLinkText;        	        	
                break;
                }
            case EAknIndicatorUSBConnection:        	
                {
        	    aTextType = EAknIndicatorPluginLinkText;
			    dynStringBuf = eikEnv->AllocReadResourceL(R_LCSTYLUSTAP_USB_NAME );            
            	break;
                }
            case EAknIndicatorUSBMemConnected:
            case EAknIndicatorUSBMemActive:   
                {     	
        	    aTextType = EAknIndicatorPluginLinkText;
			    dynStringBuf = eikEnv->AllocReadResourceL(R_LCSTYLUSTAP_USB_MEM_EJECT );            
        	    break;	        
                }
            default:
                break;
        }

		CleanupStack::PopAndDestroy(); // rLoader
        TRACE_INFO((_L("CLcStylusTap::TextL   =  %S"), dynStringBuf))
        
        return dynStringBuf;
    }

// ---------------------------------------------------------------------------
// HandleIndicatorTapL.
// Filter the aUid  and find the app plug in for the view.
// ---------------------------------------------------------------------------
//  
void CLcStylusTap::HandleIndicatorTapL( const TInt aUid )
    {	
 
    TRACE_INFO((_L("CLcStylusTap::HandleIndicatorTapL  aUid =  %d"), aUid))   
  
     switch(aUid)
 	    {    
     	case EAknIndicatorBluetooth:
     	case EAknIndicatorBluetoothVisible:
     	case EAknIndicatorBluetoothModuleOn:
     	case EAknIndicatorBluetoothModuleOnVisible:
            {                          
            TUidType uidtype(KExecutableImageUid, KUidApp,TUid::Uid(KBtUIUID));
            CreateDesiredViewL(KBTUIExe(),uidtype);     	           	
     	    break; 
            }
     	case EAknIndicatorUSBConnection:
            {					                         	            
            TUidType uidtype(KExecutableImageUid, TUid::Uid(0x00),TUid::Uid(KUsbUIUID));
            CreateDesiredViewL(KUSBExe(),uidtype);
            break;
         	}      	
     	case EAknIndicatorUSBMemConnected:
        case EAknIndicatorUSBMemActive:
            {
            EjectUSBMemL();
            break;	        
            }
 	   case EAknIndicatorIrActive: // IR not required                	
       default:
        TRACE_INFO((_L(" CLcStylusTap::HandleIndicatorTapL default")))
     	break;
 	   }                 
  	TRACE_FUNC_EXIT
    }    
  
  
// ---------------------------------------------------------------------------
// CreateDesiredViewL.
// Create the desired view via control panel.
// ---------------------------------------------------------------------------
//  
void CLcStylusTap::CreateDesiredViewL(const TDesC & aProcessName,const TUidType & aUidType) const
    {                 
    TRACE_FUNC_ENTRY
	RProcess NewProcess;	            
	User::LeaveIfError(NewProcess.Create(aProcessName, KNullDesC, aUidType));	
  	NewProcess.Resume();
	NewProcess.Close();	            		
    TRACE_FUNC_EXIT	      		
    }

// ---------------------------------------------------------------------------
// CreateDesiredViewL.
// Create the desired view via control panel.
// ---------------------------------------------------------------------------
//  
void CLcStylusTap::EjectUSBMemL()
    {                 
    TRACE_FUNC_ENTRY
    delete iDismountManager;
    iDismountManager = NULL;
    iDismountManager= CLcStylusTapDismount::NewL();
    iDismountManager->DisMountUsbDrives();    
    TRACE_FUNC_EXIT	      		
    }
    


// End of File
