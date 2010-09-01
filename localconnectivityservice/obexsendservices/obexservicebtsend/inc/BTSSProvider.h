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
* Description:  
*
*/



#ifndef BT_SENDING_SERVICE_PROVIDER_H
#define BT_SENDING_SERVICE_PROVIDER_H

//  INCLUDES
#include <AiwServiceIfMenu.h>

// FORWARD DECLARATIONS
class CBTServiceParameterList;
class CBTServiceAPI;
class CBTSSSendListHandler;

// CLASS DECLARATION

/**
*  Provides BT sending services
*  
*/
class CBTSSProvider : public CAiwServiceIfMenu
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CBTSSProvider* NewL();
        
        /**
        * Destructor.
        */
        virtual ~CBTSSProvider();


    public: // Functions from base classes

        /**
        * From CAiwServiceIfBase    Not used
        * @since 2.6
        * @param aFrameworkCallback Not used
        * @param aInterest          Not used
        * @return None
        */
        virtual void InitialiseL( MAiwNotifyCallback& aFrameworkCallback,
			                      const RCriteriaArray& aInterest );

        /**
        * From CAiwServiceIfBase Processes service command
        * and starts bt sending
        * @since 2.6
        * @param aCmdId         Command id
        * @param aInParamList   Not used
        * @param aOutParamList  List of files
        * @param aCmdOptions    Not used
        * @param aCallback      Not used
        * @return None
        */
		virtual void HandleServiceCmdL( const TInt& aCmdId, 
                                        const CAiwGenericParamList& aInParamList,
                                        CAiwGenericParamList& aOutParamList,
                                        TUint aCmdOptions,
                                        const MAiwNotifyCallback* aCallback );
                                        
        /**
        * From CAiwServiceIfBase Processes service command
        * and starts bt sending
        * @since 2.6
        * @param aCmdId         Command id
        * @param aInParamList   Not used
        * @param aOutParamList  List of files
        * @param aCmdOptions    Not used
        * @param aCallback      Not used
        * @return None
        */                                
        virtual void HandleMenuCmdL(TInt aMenuCmdId, 
                                    const CAiwGenericParamList& aInParamList,
                                    CAiwGenericParamList& aOutParamList,
                                    TUint aCmdOptions,
                                    const MAiwNotifyCallback* aCallback );
                                    
                                    
                                    
        virtual  void InitializeMenuPaneL(  CAiwMenuPane& aMenuPane,
                                            TInt aIndex,
                                            TInt /* aCascadeId */,
                                            const CAiwGenericParamList& aInParamList );

    private:

        /**
        * C++ default constructor.
        */
        CBTSSProvider();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:    // Data

        CBTServiceAPI*          iBTSendingService;
        
        CBTSSSendListHandler*   iConverter; 
    };

#endif      // BT_SENDING_SERVICE_PROVIDER_H
            
// End of File
