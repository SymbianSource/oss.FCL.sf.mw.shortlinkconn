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
* Description:  Header file for LcStylusTapTest
*
*/




#ifndef LcStylusTapTest_H
#define LcStylusTapTest_H

//  INCLUDES
#include <StifLogger.h>
#include <TestScripterInternal.h>
#include <StifTestModule.h>
#include <TestclassAssert.h>
#include <ecom.h>
#include <AknIndicatorPlugin.h>
#include "lcstylustap.h"

// CONSTANTS
//const ?type ?constant_var = ?constant;

// MACROS
//#define ?macro ?macro_def
#define TEST_CLASS_VERSION_MAJOR 0
#define TEST_CLASS_VERSION_MINOR 0
#define TEST_CLASS_VERSION_BUILD 0

#ifdef STIF_LOG
#undef STIF_LOG
#endif

#define STIF_LOG( s )\
    {\
    TBuf<KMaxLogData> traceBuf;\
    traceBuf.Append( _L( "[STIF_LOG] " ) );\
    traceBuf.Append( _L( s ) );\
    iLog->Log( _L( s ) );\
    RDebug::Print( traceBuf );\
    }

#define STIF_LOG1( s, v ) \
    {\
    TBuf<KMaxLogData> traceBuf;\
    traceBuf.Append( _L( "[STIF_LOG] " ) );\
    traceBuf.Append( _L( s ) );\
    iLog->Log( _L( s ), v );\
    RDebug::Print( traceBuf, v );\
    }

#define STIF_LOG2( s, v1, v2 ) \
    {\
    TBuf<KMaxLogData> traceBuf;\
    traceBuf.Append( _L( "[STIF_LOG] " ) );\
    traceBuf.Append( _L( s ) );\
    iLog->Log( _L( s ), v1, v2 );\
    RDebug::Print( traceBuf, v1, v2 );\
    }

#define STIF_LOG3( s, v1, v2, v3 ) \
    {\
    TBuf<KMaxLogData> traceBuf;\
    traceBuf.Append( _L( "[STIF_LOG] " ) );\
    traceBuf.Append( _L( s ) );\
    iLog->Log( _L( s ), v1, v2, v3 );\
    RDebug::Print( traceBuf, v1, v2, v3 );\
    }

// Logging path
_LIT( KLcStylusTapTestLogPath, "\\logs\\testframework\\LcStylusTapTest\\" );

// Logging path for ATS - for phone builds comment this line
//_LIT( KLcStylusTapTestLogPath, "e:\\testing\\stiflogs\\" ); 

// Log file
_LIT( KLcStylusTapTestLogFile, "LcStylusTapTest.txt" ); 
_LIT( KLcStylusTapTestLogFileWithTitle, "LcStylusTapTest_[%S].txt" );

// FUNCTION PROTOTYPES
//?type ?function_name(?arg_list);

// FORWARD DECLARATIONS
//class ?FORWARD_CLASSNAME;
class CLcStylusTapTest;
class CLcStylusTap;

// DATA TYPES
//enum ?declaration

enum TLcStylusTapTestResult
    {
    ETestCasePassed,
    ETestCaseFailed
    };

//typedef ?declaration
//extern ?data_type;

// CLASS DECLARATION

NONSHARABLE_CLASS( TLcStylusTapTestBlockParams )
    {
    public:
        TPtrC iTestBlockName;
        
        TPtrC iTestOption1;
        TPtrC iTestOption2;
        TPtrC iTestOption3;
        
        TInt iTestIntOption1;
        TInt iTestIntOption2;
        
        TChar iTestCharOption1;
        TChar iTestCharOption2;
    };

/**
*  CLcStylusTapTest test class for STIF Test Framework TestScripter.
*  ?other_description_lines
*
*  @lib ?library
*  @since ?Series60_version
*/
NONSHARABLE_CLASS( CLcStylusTapTest ) : public CScriptBase
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CLcStylusTapTest* NewL( CTestModuleIf& aTestModuleIf );

        /**
        * Destructor.
        */
        virtual ~CLcStylusTapTest();

    public: // New functions

        /**
        * ?member_description.
        * @since ?Series60_version
        * @param ?arg1 ?description
        * @return ?description
        */
        //?type ?member_function( ?type ?arg1 );

    public: // Functions from base classes

        /**
        * From CScriptBase Runs a script line.
        * @since ?Series60_version
        * @param aItem Script line containing method name and parameters
        * @return Symbian OS error code
        */
        virtual TInt RunMethodL( CStifItemParser& aItem );

    protected:  // New functions

        /**
        * ?member_description.
        * @since ?Series60_version
        * @param ?arg1 ?description
        * @return ?description
        */
        //?type ?member_function( ?type ?arg1 );

    protected:  // Functions from base classes

        /**
        * From ?base_class ?member_description
        */
        //?type ?member_function();

    private:

        /**
        * C++ default constructor.
        */
        CLcStylusTapTest( CTestModuleIf& aTestModuleIf );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        // Prohibit copy constructor if not deriving from CBase.
        // ?classname( const ?classname& );
        // Prohibit assigment operator if not deriving from CBase.
        // ?classname& operator=( const ?classname& );

        /**
        * Frees all resources allocated from test methods.
        * @since ?Series60_version
        */
        void Delete();

        /**
        * Test methods are listed below. 
        */

        virtual TInt ExecuteApiTestBlock( CStifItemParser& aItem );
        virtual TInt ExecuteModuleTestBlock( CStifItemParser& aItem );
        virtual TInt ExecuteBranchTestBlock( CStifItemParser& aItem );
        
        /**
         * Method used to log version of test class
         */
        void SendTestClassVersion();

        //ADD NEW METHOD DEC HERE
        //[TestMethods] - Do not remove

        void GetTestBlockParamsL( CStifItemParser& aItem );
        
    	void DoExecuteApiTestBlockL( CStifItemParser& aItem, TLcStylusTapTestResult& aTestResult );    	
    	void DoExecuteModuleTestBlockL( CStifItemParser& aItem, TLcStylusTapTestResult& aTestResult );    
    	void DoExecuteBranchTestBlockL( CStifItemParser& aItem, TLcStylusTapTestResult& aTestResult );
    	
        void ExampleTestL( TPtrC aTestOption, TPtrC aTestSubOption, 
                 TInt aTestIntOption, TInt aTestCharOption, TLcStylusTapTestResult& aTestResult );
        
        //tests method
        void CreateLcStylusTapTestL( TPtrC aTestOption, TPtrC aTestSubOption, TLcStylusTapTestResult& aTestResult );
        
        void TextTapTestL( TPtrC aTestOption, TPtrC aTestSubOption, TLcStylusTapTestResult& aTestResult );
        
        void CallHandleIndicatorTapTestL( TPtrC aTestOption, TPtrC aTestSubOption, TLcStylusTapTestResult& aTestResult );
        
        //other methods
        TBool GetPluginImplementation(TInt aValue);        
        void DestroyImplementation();
        //cheks if TextL method of lcstylustap returns correct values 
        TInt CheckTextResult(TInt aValue, HBufC* &aMedia); 
	
    public:     // Data
        // ?one_line_short_description_of_data
        //?data_declaration;

    protected:  // Data
        // ?one_line_short_description_of_data
        //?data_declaration;

    private:    // Data
        TLcStylusTapTestBlockParams iTestBlockParams;
        
        CLcStylusTap* iLcStylusTap;        

    	RImplInfoPtrArray   iPluginImpArray;
        TUid iDtor_Key;
		TInt iTextType;		
        RImplInfoPtrArray iImplementations;
        
        // Reserved pointer for future extension
        //TAny* iReserved;

    public:     // Friend classes
        //?friend_class_declaration;
    protected:  // Friend classes
        //?friend_class_declaration;
    private:    // Friend classes
        //?friend_class_declaration;
        
    };

#endif      // LcStylusTapTest_H

// End of File
