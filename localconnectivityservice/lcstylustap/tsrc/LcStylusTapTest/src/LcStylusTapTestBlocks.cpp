/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Implementation
*
*/




// [INCLUDE FILES] - do not remove
#include <e32svr.h>
#include <StifParser.h>
#include <Stiftestinterface.h>
#include <avkon.hrh>
//tests headers
#include "LcStylusTapTest.h"

// EXTERNAL DATA STRUCTURES
//extern  ?external_data;

// EXTERNAL FUNCTION PROTOTYPES  
//extern ?external_function( ?arg_type,?arg_type );

// CONSTANTS
//const ?type ?constant_var = ?constant;
_LIT(KBlueTooth, "Bluetooth");
_LIT(KUSB, "USB");
_LIT(KIRDA, "Infrared");

// MACROS
//#define ?macro ?macro_def

// LOCAL CONSTANTS AND MACROS
//const ?type ?constant_var = ?constant;
//#define ?macro_name ?macro_def

// MODULE DATA STRUCTURES
//enum ?declaration
//typedef ?declaration

// LOCAL FUNCTION PROTOTYPES
//?type ?function_name( ?arg_type, ?arg_type );

// FORWARD DECLARATIONS
//class ?FORWARD_CLASSNAME;

// ============================= LOCAL FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// ?function_name ?description.
// ?description
// Returns: ?value_1: ?description
//          ?value_n: ?description_line1
//                    ?description_line2
// -----------------------------------------------------------------------------
//
/*
?type ?function_name(
    ?arg_type arg,  // ?description
    ?arg_type arg)  // ?description
    {

    ?code  // ?comment

    // ?comment
    ?code
    }
*/

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CLcStylusTapTest::Delete
// Delete here all resources allocated and opened from test methods. 
// Called from destructor. 
// -----------------------------------------------------------------------------
//
void CLcStylusTapTest::Delete() 
    {
    
    }

// -----------------------------------------------------------------------------
// CLcStylusTapTest::RunMethodL
// Run specified method. Contains also table of test mothods and their names.
// -----------------------------------------------------------------------------
//
TInt CLcStylusTapTest::RunMethodL( 
    CStifItemParser& aItem ) 
    {

    static TStifFunctionInfo const KFunctions[] =
        {  
        //ADD NEW ENTRY HERE
        // [test cases entries] - Do not remove
		ENTRY( "ExecuteApiTestBlock", CLcStylusTapTest::ExecuteApiTestBlock ),
        ENTRY( "ExecuteModuleTestBlock", CLcStylusTapTest::ExecuteModuleTestBlock ),
        ENTRY( "ExecuteBranchTestBlock", CLcStylusTapTest::ExecuteBranchTestBlock ),
        };

    const TInt count = sizeof( KFunctions ) / 
                        sizeof( TStifFunctionInfo );

    return RunInternalL( KFunctions, count, aItem );

    }

// -----------------------------------------------------------------------------
// CLcStylusTapTest::GetTestBlockParamsL
// -----------------------------------------------------------------------------

void CLcStylusTapTest::GetTestBlockParamsL( CStifItemParser& aItem )
    {
    STIF_LOG( ">>> GetTestBlockParamsL" );
    
    // Add new test block branches below, get all required test parameters    
    if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ExampleTestL" ) ) )
        {              
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );        
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption2 ) );
        User::LeaveIfError( aItem.GetNextInt( iTestBlockParams.iTestIntOption1 ) );        
        User::LeaveIfError( aItem.GetNextChar( iTestBlockParams.iTestCharOption1 ) );        
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CreateLcStylusTap" ) ) )
		{              
		User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption2 ) );
		}
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "TextTapTest" ) ) )
		{              
		User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );       
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption2 ) );
		}
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CallHandleIndicatorTap" ) ) )
		{              
		User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption1 ) );      
        User::LeaveIfError( aItem.GetNextString( iTestBlockParams.iTestOption2 ) );
		}
    else//
        {
        STIF_LOG( "Test type: not found" );
        User::Leave( KErrNotFound );
        }
    STIF_LOG( "<<< GetTestBlockParamsL" );
    }

// -----------------------------------------------------------------------------
// CLcStylusTapTest::ExecuteApiTestBlock
// -----------------------------------------------------------------------------

TInt CLcStylusTapTest::ExecuteApiTestBlock( CStifItemParser& aItem )
    {
	STIF_LOG( ">>>ExecuteApiTestBlock" );
	
	TInt res;
    TLcStylusTapTestResult testResult = ETestCaseFailed;
	
    TRAP( res, DoExecuteApiTestBlockL( aItem, testResult ) );
    if ( res != KErrNone )
        {
        STIF_LOG1( "DoExecuteApiTestBlockL error: %d", res );
        return res;
        }
    
    STIF_ASSERT_EQUALS( ETestCasePassed, testResult );
    STIF_LOG( "Test case passed" );
	STIF_LOG( "<<<ExecuteApiTestBlock" );
	
    return KErrNone;
    }
	
	
void CLcStylusTapTest::DoExecuteApiTestBlockL( CStifItemParser& aItem, TLcStylusTapTestResult& aTestResult )
    {
	STIF_LOG( ">>>DoExecuteApiTestBlockL" );

	User::LeaveIfError( aItem.GetString( _L( "ExecuteApiTestBlock" ), iTestBlockParams.iTestBlockName ) );
	STIF_LOG1( "Api test type: %S", &iTestBlockParams.iTestBlockName );
	
	GetTestBlockParamsL( aItem );
	
	// Add new API test block branches with optional test parameters here	
    if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ExampleTestL" ) ) )
        {      
        ExampleTestL( iTestBlockParams.iTestOption1, iTestBlockParams.iTestOption2, 
                iTestBlockParams.iTestIntOption1, iTestBlockParams.iTestCharOption1, aTestResult );
        }	
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CreateLcStylusTap" ) ) )
        {      
        CreateLcStylusTapTestL( iTestBlockParams.iTestOption1, iTestBlockParams.iTestOption2, aTestResult );
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "TextTapTest" ) ) )
        {      
        TextTapTestL( iTestBlockParams.iTestOption1, iTestBlockParams.iTestOption2, aTestResult );
        }
    else if ( !iTestBlockParams.iTestBlockName.Compare( _L( "CallHandleIndicatorTap" ) ) )
        {      
        CallHandleIndicatorTapTestL( iTestBlockParams.iTestOption1, iTestBlockParams.iTestOption2, aTestResult );
        }
    else
        {
        STIF_LOG( "Test type: not found" );
        User::Leave( KErrNotFound );
        }
	
	STIF_LOG( "<<<DoExecuteApiTestBlockL" );
    }
	
// -----------------------------------------------------------------------------
// CLcStylusTapTest::ExecuteModuleTestBlock
// -----------------------------------------------------------------------------	

TInt CLcStylusTapTest::ExecuteModuleTestBlock( CStifItemParser& aItem )
    {
	STIF_LOG( "[STIF_LOG] >>>ExecuteModuleTestBlock" );
	
    TInt res;
    TLcStylusTapTestResult testResult;
    
    TRAP( res, DoExecuteModuleTestBlockL( aItem, testResult ) );
    if ( res != KErrNone )
        {
        STIF_LOG1( "DoExecuteModuleTestBlockL error: %d", res );
        return res;
        }
    
    STIF_ASSERT_EQUALS( ETestCasePassed, testResult );
    STIF_LOG( "[STIF_LOG] Test case passed" );
	STIF_LOG( "[STIF_LOG] <<<ExecuteModuleTestBlock" );
    return KErrNone;
    }	
	
	
void CLcStylusTapTest::DoExecuteModuleTestBlockL( CStifItemParser& aItem, TLcStylusTapTestResult& aTestResult )
    {
	STIF_LOG( "[STIF_LOG] >>>DoExecuteModuleTestBlockL" );
	
    User::LeaveIfError( aItem.GetString( _L( "ExecuteModuleTestBlock" ), iTestBlockParams.iTestBlockName ) );
    STIF_LOG1( "Module test type: %S", &iTestBlockParams.iTestBlockName );
    
    GetTestBlockParamsL( aItem );
    
    // Add new module test block branches with optional test parameters here   
    if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ExampleTestL" ) ) )
        {      
        ExampleTestL( iTestBlockParams.iTestOption1, iTestBlockParams.iTestOption2, 
                iTestBlockParams.iTestIntOption1, iTestBlockParams.iTestCharOption1, aTestResult );
        }
    else
        {
        STIF_LOG( "Test type: not found" );
        User::Leave( KErrNotFound );
        }
    
	STIF_LOG( "[STIF_LOG] <<<DoExecuteModuleTestBlockL" );
    }
	
// -----------------------------------------------------------------------------
// CLcStylusTapTest::ExecuteBranchTestBlock
// -----------------------------------------------------------------------------
	
TInt CLcStylusTapTest::ExecuteBranchTestBlock( CStifItemParser& aItem )
    {
	STIF_LOG( "[STIF_LOG] >>>ExecuteBranchTestBlock" );
	
    TInt res;
    TLcStylusTapTestResult testResult;
    
    TRAP( res, DoExecuteBranchTestBlockL( aItem, testResult ) );
    if ( res != KErrNone )
        {
        STIF_LOG1( "DoExecuteBranchTestBlockL error: %d", res );
        return res;
        }   
    
    STIF_ASSERT_EQUALS( ETestCasePassed, testResult );
    STIF_LOG( "[STIF_LOG] Test case passed" );
	STIF_LOG( "[STIF_LOG] <<<ExecuteBranchTestBlock" );
    return KErrNone;
    }

	
void CLcStylusTapTest::DoExecuteBranchTestBlockL( CStifItemParser& aItem, TLcStylusTapTestResult& aTestResult )
    {
	STIF_LOG( "[STIF_LOG] >>>DoExecuteBranchTestBlockL" );
	
    User::LeaveIfError( aItem.GetString( _L( "ExecuteBranchTestBlock" ), iTestBlockParams.iTestBlockName ) );
    STIF_LOG1( "Branch test type: %S", &iTestBlockParams.iTestBlockName );
    
    GetTestBlockParamsL( aItem );
    
    // Add new branch test block branches with optional test parameters here   
    if ( !iTestBlockParams.iTestBlockName.Compare( _L( "ExampleTestL" ) ) )
        {      
        ExampleTestL( iTestBlockParams.iTestOption1, iTestBlockParams.iTestOption2, 
                iTestBlockParams.iTestIntOption1, iTestBlockParams.iTestCharOption1, aTestResult );
        }
    else
        {
        STIF_LOG( "Test type: not found" );
        User::Leave( KErrNotFound );
        }
    
	STIF_LOG( "[STIF_LOG] <<<DoExecuteBranchTestBlockL" );
    }

// Add test block methods implementation here
// -----------------------------------------------------------------------------
// CLcStylusTapTest::ExampleTestL
// -----------------------------------------------------------------------------

void CLcStylusTapTest::ExampleTestL( TPtrC aTestOption, TPtrC aTestSubOption, 
        TInt aTestIntOption, TInt aTestCharOption, TLcStylusTapTestResult& aTestResult )
    {
    STIF_LOG( ">>>ExampleTestL" );
    
    if ( !aTestOption.Compare( _L( "API" ) ) )
        {
        STIF_LOG1( "Api test option: %S", &aTestOption );
        STIF_LOG1( "Api test sub-option: %S", &aTestSubOption );
        STIF_LOG1( "Api test int option: %d", aTestIntOption );
        STIF_LOG1( "Api test char option: %c", aTestCharOption );
        }
    else if ( !aTestOption.Compare( _L( "MODULE" ) ) )
        {
        STIF_LOG1( "Module test option: %S", &aTestOption );
        STIF_LOG1( "Module test sub-option: %S", &aTestSubOption );
        STIF_LOG1( "Module test int option: %d", aTestIntOption );
        STIF_LOG1( "Module test char option: %c", aTestCharOption );
        }
    else if ( !aTestOption.Compare( _L( "BRANCH" ) ) )
        {
        STIF_LOG1( "Branch test option: %S", &aTestOption );
        STIF_LOG1( "Branch test sub-option: %S", &aTestSubOption );
        STIF_LOG1( "Branch test int option: %d", aTestIntOption );
        STIF_LOG1( "Branch test char option: %c", aTestCharOption );
        }
    else
        {
        STIF_LOG( "Invalid test parameter" );
        User::Leave( KErrNotFound );
        }
    
    aTestResult = ETestCasePassed;
    
    STIF_LOG( "<<<ExampleTestL" );
    }

// -----------------------------------------------------------------------------
// CLcStylusTapTest::CreateLcStylusTapTestL
// -----------------------------------------------------------------------------
//
void CLcStylusTapTest::CreateLcStylusTapTestL( TPtrC aTestOption, TPtrC aTestSubOption, TLcStylusTapTestResult& aTestResult )
    {
    STIF_LOG( ">>>CreateLcStylusTapL" );    	
    
    delete iLcStylusTap;
    iLcStylusTap = NULL;
    
    if( !aTestSubOption.Compare(_L( "BT" )) )
		{
		GetPluginImplementation(EAknIndicatorBluetoothModuleOnVisible);
		STIF_LOG1( "CreateLcStylusTapL BT result: %d", iLcStylusTap ? 1 : 0 );
		}
    else if( !aTestSubOption.Compare(_L( "USB" )) )
		{
		GetPluginImplementation(EAknIndicatorUSBConnection);
		STIF_LOG1( "CreateLcStylusTapL USB result: %d", iLcStylusTap ? 1 : 0 );
		}
    else if( !aTestSubOption.Compare(_L( "IRDA" )) )
		{
		GetPluginImplementation(EAknIndicatorIrActive);
		STIF_LOG1( "CreateLcStylusTapL IR result: %d", iLcStylusTap ? 1 : 0 );
		}
    else if( !aTestSubOption.Compare(_L( "USBMEM" )) )
		{
		GetPluginImplementation(EAknIndicatorUSBMemConnected);
		STIF_LOG1( "CreateLcStylusTapL USB result: %d", iLcStylusTap ? 1 : 0 );
		}
    else
    	{}
    
    if(!iLcStylusTap)
    	{
    	STIF_LOG( "Error iLcStylusTap == NULL" );
		return;
    	}	
    
    DestroyImplementation();
    
    aTestResult = ETestCasePassed;
    
    STIF_LOG( "<<<CreateLcStylusTapL" );
    }

// -----------------------------------------------------------------------------
// CLcStylusTapTest::TextTapTestL
// -----------------------------------------------------------------------------
//
void CLcStylusTapTest::TextTapTestL( TPtrC aTestOption, TPtrC aTestSubOption, TLcStylusTapTestResult& aTestResult )
	{
	STIF_LOG(">>CLcStylusTapTest::TextTapTestL");
	
	HBufC* Media = NULL;
	delete iLcStylusTap;
	iLcStylusTap = NULL;
	
    if( !aTestSubOption.Compare(_L( "BT" )) )
		{
		GetPluginImplementation(EAknIndicatorBluetoothModuleOnVisible);
		if(iLcStylusTap)
			{			
			Media = iLcStylusTap->TextL(EAknIndicatorBluetoothModuleOnVisible, iTextType);
			STIF_LOG1( "CheckTextResult err: %d", CheckTextResult(EAknIndicatorBluetoothModuleOnVisible, Media) );
			STIF_LOG1( "TextL: %S, OK", Media );
			}
		}
    else if( !aTestSubOption.Compare(_L( "USB" )) )
		{
		GetPluginImplementation(EAknIndicatorUSBConnection);
		if(iLcStylusTap)
			{			
			Media = iLcStylusTap->TextL(EAknIndicatorUSBConnection, iTextType);
			STIF_LOG1( "TextL: %S, OK", Media );
			STIF_LOG1( "CheckTextResult err: %d", CheckTextResult(EAknIndicatorUSBConnection, Media) );
			}
		}
    else if( !aTestSubOption.Compare(_L( "USBMEM" )) )
		{
		GetPluginImplementation(EAknIndicatorUSBMemConnected);
		if(iLcStylusTap)
			{			
			Media = iLcStylusTap->TextL(EAknIndicatorUSBMemConnected, iTextType);
			STIF_LOG1( "TextL: %S no checked", Media );
			//STIF_LOG1( "CheckTextResult err: %d", CheckTextResult(EAknIndicatorUSBMemConnected, Media) );
			}
		}
    else if( !aTestSubOption.Compare(_L( "USBMEM_loc" )) )
		{
		GetPluginImplementation(EAknIndicatorUSBMemConnected);
		if(iLcStylusTap)
			{			
			Media = iLcStylusTap->TextL(EAknIndicatorUSBMemConnected, iTextType);
			STIF_LOG1( "CheckTextResult err: %d", CheckTextResult(EAknIndicatorUSBMemConnected, Media) );
			STIF_LOG1( "TextL: %S, OK", Media );
			}
		}
    else
        User::LeaveIfError(KErrNotFound);

    DestroyImplementation();
    
    aTestResult = ETestCasePassed;
    
	STIF_LOG("<<CallHandleIndicatorTapL::TextTapTestL");
	}
	
// -----------------------------------------------------------------------------
// CLcStylusTapTest::CallHandleIndicatorTapTestL
// -----------------------------------------------------------------------------
//
void CLcStylusTapTest::CallHandleIndicatorTapTestL( TPtrC aTestOption, TPtrC aTestSubOption, TLcStylusTapTestResult& aTestResult )
	{
	STIF_LOG(">>CLcStylusTapTest::CallHandleIndicatorTapL");
	
    if( !aTestSubOption.Compare(_L( "BT" )) )
		{
		GetPluginImplementation(EAknIndicatorBluetoothModuleOnVisible);
		if(iLcStylusTap)
			{
			iLcStylusTap->HandleIndicatorTapL(EAknIndicatorBluetoothModuleOnVisible);
			STIF_LOG("HandleIndicatorTapL OK");
			}
		}
    else if( !aTestSubOption.Compare(_L( "USB" )) )
		{
		GetPluginImplementation(EAknIndicatorUSBConnection);
		if(iLcStylusTap)
			{
			iLcStylusTap->HandleIndicatorTapL(EAknIndicatorUSBConnection);
			STIF_LOG("HandleIndicatorTapL OK");
			}
		}
    else if( !aTestSubOption.Compare(_L( "USBMEM" )) )
		{
		GetPluginImplementation(EAknIndicatorUSBMemConnected);
		if(iLcStylusTap)
			{
			iLcStylusTap->HandleIndicatorTapL(EAknIndicatorUSBMemConnected);
			STIF_LOG("HandleIndicatorTapL OK");
			}
		}
    else if( !aTestSubOption.Compare(_L( "IRDA" )) )
		{
		GetPluginImplementation(EAknIndicatorIrActive);
		if(iLcStylusTap)
			{
			iLcStylusTap->HandleIndicatorTapL(EAknIndicatorIrActive);
			STIF_LOG("HandleIndicatorTapL OK");
			}
		}
    else
        User::LeaveIfError(KErrNotFound);

    DestroyImplementation();
    
    aTestResult = ETestCasePassed;
       	
	STIF_LOG(">>CLcStylusTapTest::CallHandleIndicatorTapL");
	}

// Other operaton functions
// -----------------------------------------------------------------------------
// CLcStylusTapTest::GetPluginImplementation
// returns CLcStylusTap pointer
// -----------------------------------------------------------------------------
//
TBool CLcStylusTapTest::GetPluginImplementation(TInt aValue)
	{
	STIF_LOG( ">>CLcStylusTapTest::GetPluginImplementation" );
	
		
	const TUid uidInterfacetobepop = TUid::Uid(KAknIndicatorPluginInterfaceUid);	  	
	TRAPD(err, REComSession::ListImplementationsL(uidInterfacetobepop, iImplementations));
	STIF_LOG1( "REComSession::ListImplementationsL err: %d", err );
	if(err != KErrNone)
		User::LeaveIfError(err);
	 
	delete iLcStylusTap;
	iLcStylusTap = NULL;
	//HBufC* Media = NULL;
		
	TUid btimpluid = {KImplUIDBTIndicatorsPlugin};
	TUid usbimpluid = {KImplUIDUSBIndicatorsPlugin};
	TUid irimpluid = {KImplUIDIRIndicatorsPlugin};
	TUid usbMemImp = {0x20026FC4};//usb_mem
	
	switch (aValue)
		{
		case EAknIndicatorBluetooth://12
		case EAknIndicatorBluetoothVisible:
		case EAknIndicatorBluetoothModuleOn:
		case EAknIndicatorBluetoothModuleOnVisible://55
			iLcStylusTap = REINTERPRET_CAST(CLcStylusTap*, REComSession::CreateImplementationL(btimpluid, iDtor_Key) );   
			break;
		case EAknIndicatorUSBConnection:      //28  
			iLcStylusTap = REINTERPRET_CAST(CLcStylusTap*, REComSession::CreateImplementationL(usbimpluid, iDtor_Key) );   
			break;
		case EAknIndicatorIrActive:   
			iLcStylusTap = REINTERPRET_CAST(CLcStylusTap*, REComSession::CreateImplementationL(irimpluid, iDtor_Key) );   
			break;
		case EAknIndicatorUSBMemConnected:
        case EAknIndicatorUSBMemActive:      
			iLcStylusTap = REINTERPRET_CAST(CLcStylusTap*, REComSession::CreateImplementationL(usbMemImp, iDtor_Key) );   
			break;		        
		default:         
			STIF_LOG( "GetPluginImplementation ERROR!" ); 
			User::LeaveIfError(KErrNotFound);
			break;
		}
	   
	STIF_LOG1( "iLcStylusTap: %d", iLcStylusTap ? 1 : 0 );  
	    
    return ETrue;
	}


// -----------------------------------------------------------------------------
// CLcStylusTapTest::CheckTextResult
// -------------------------------------------
//
TInt CLcStylusTapTest::CheckTextResult(TInt aValue, HBufC* &aMedia)
	{
	TInt err = KErrNone;
	
	if(!aMedia)
		{		
		User::Leave(KErrNotFound);
		}
	
	switch(aValue)
		{
		case EAknIndicatorBluetooth://12
		case EAknIndicatorBluetoothVisible:
		case EAknIndicatorBluetoothModuleOn:
		case EAknIndicatorBluetoothModuleOnVisible://55
			if(aMedia->Compare(KBlueTooth())!= 0 || iTextType != CAknIndicatorPlugin::EAknIndicatorPluginLinkText)
				err = KErrNotFound;
			break;
		case EAknIndicatorUSBConnection: 
			if(aMedia->Compare(KUSB())!= 0 || iTextType != CAknIndicatorPlugin::EAknIndicatorPluginLinkText)
				err = KErrNotFound;
			break;
		case EAknIndicatorIrActive:
			if(aMedia->Compare(KIRDA())!= 0 || iTextType != CAknIndicatorPlugin::EAknIndicatorPluginLinkText)
				err = KErrNotFound;
			break;
		default:
			err = KErrNotFound;
			break;
		}
	
	if(err == KErrNotFound)
		{
		delete aMedia;
		aMedia = NULL;
		User::LeaveIfError(KErrNotFound); // Did not match that should
		}
	
	STIF_LOG("CallHandleIndicatorTapL::CompareMedia OK");
	
	return err;
	}
// -----------------------------------------------------------------------------
// CLcStylusTapTest::DestroyImplementation
// destroy plugin implementation
// -------------------------------------------
//
void CLcStylusTapTest::DestroyImplementation()
	{
	STIF_LOG( ">>CLcStylusTapTest::DestroyImplementation" );
    
    if(iLcStylusTap)
    	{
    	delete iLcStylusTap;
    	iLcStylusTap = NULL;
    	}

   	iImplementations.ResetAndDestroy();
   	iImplementations.Close();   	
   	
    REComSession::DestroyedImplementation(iDtor_Key);
    
    STIF_LOG("<<CLcStylusTapTest::DestroyImplementation");
	}


// ========================== OTHER EXPORTED FUNCTIONS =========================
// None

//  [End of File] - Do not remove
