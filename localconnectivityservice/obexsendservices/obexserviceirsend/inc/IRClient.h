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
* Description:  Obex client
*
*/



#ifndef IR_SERVICE_CLIENT_H
#define IR_SERVICE_CLIENT_H

//  INCLUDES


#include <obex.h>
#include <obexutilsdialog.h>
#include <AiwServiceHandler.h>


// FORWARD DECLARATION


// CLASS DECLARATION

// CLASS DECLARATION

/**
*  An active object managing the Obex client.
*/
class CIRClient : public CActive, 
                  MObexUtilsProgressObserver,
                  MObexUtilsDialogObserver                         
                         
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.        
        * @return None.
        */
        static CIRClient* NewL();
        
        /**
        * Destructor.
        */
        virtual ~CIRClient();

    public: // New functions        
        

        /**
        * Issues an Obex Put-request.        
        * @param aHeaderList The headers to be associated with the object.
        * @param aFileName A filename of the Put Object.
        * @return None.
        */
        void PutObjectL( );

        /**
        * Closes Obex Client connection.
        * @param None.
        * @return None.
        */
        void CloseClientConnection();
        
        
        void StartSendL( const CAiwGenericParamList& aOutParamList );

    private: // Functions from base classes

        /**
        * From MBTServiceProgressGetter Returns the progess status.
        * @return The number of bytes sent.
        */
        TInt GetProgressStatus();
        
        void DialogDismissed( TInt aButtonId );
        
        void PutCompleted();

        
    private: // Functions from base classes
        

        /**
        * From CActive Get's called when a request is cancelled.
        * @param None.
        * @return None.
        */
        void DoCancel();

        /**
        * From CActive Get's called when a request is completed.
        * @param None.
        * @return None.
        */
	    void RunL();
	    
	    void AddFileHandleL(RFile aFile);
	    
	    void AddFileL(const TDesC& aFilePath);
	    
	    void SendL();
	    
	    TInt FileListsize();
	    
	    void ShowNote();

    private:    // Data definitions

        enum TIRServiceClientState
            {            
            EIRCliConnecting,
            EIRCliPutting,
            EIRCliGetting,
            EIRCliDisconnected
            };

    private:

        /**
        * C++ default constructor.
        */
        CIRClient( );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:    // Data

        TIRServiceClientState       iClientState;

        CObexClient*                iClient;
        CBufFlat*                   iObjectBuffer;
        CObexBufObject*             iPutBufObject;        
        CObexNullObject*            iConnectObject;
        TInt                        iTotalBytesSent;        
        TInt                        iObjectIndex;
        RArray<RFile>               iFileArray;
        RFs                         iFileSession;
        CBufFlat*                   iBuffer;
        CObexUtilsDialog*           iDialog;
        

        // Not owned
        //
        
    };

#endif      // BT_SERVICE_CLIENT_H
            
// End of File
