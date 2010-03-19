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
* Description:  Obex client implementation
*
*/



// INCLUDE FILES
#include <obexclient.h>
#include <Obexutils.rsg>
#include <obexutilsuilayer.h>

#include "IRClient.h"
#include "IrSendingServiceDebug.h"


const TUint KIRProgressInterval         = 1000000;

_LIT( KTransportTinyTp, "IrTinyTP" );
_LIT8( KClassNameObex, "OBEX" );
_LIT8( KAttName, "IrDA:TinyTP:LsapSel" );

// CONSTANTS

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBTServiceClient::CBTServiceClient
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CIRClient::CIRClient(  ) 
    : CActive( EPriorityStandard ), 
      iClientState( EIRCliDisconnected),                             
      iObjectIndex( 0 )
    {
    CActiveScheduler::Add( this );    
    }

// -----------------------------------------------------------------------------
// CBTServiceClient::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CIRClient::ConstructL( )
    {
    FLOG(_L("[BTSU]\t CBTServiceClient::ConstructL()"));

    User::LeaveIfError( iFileSession.Connect() );
    
    iDialog = CObexUtilsDialog::NewL( this );
    iTotalBytesSent = 0;
    // Create Obex Client
    //
    TObexIrProtocolInfo  info;
    //info.iTransport = KObexIrTTPProtocol;    
    
  //  info.iAddr.SetPort( KDefaultObexPort );//default obex server for now
    info.iTransport     = KTransportTinyTp;
    info.iClassName     = KClassNameObex;
    info.iAttributeName = KAttName;

    
    iClient = CObexClient::NewL( info );
    

    // Create Connect-object
    //
    iConnectObject = CObexNullObject::NewL();

    //Show note
    //
    iDialog->LaunchWaitDialogL( R_IR_CONNECTING_WAIT_NOTE);
    
    // Establish client connection
    //
    iClient->Connect( /**iConnectObject,*/ iStatus );
    SetActive();
    iClientState = EIRCliConnecting;

    FLOG(_L("[BTSU]\t CBTServiceClient::ConstructL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceClient::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CIRClient* CIRClient::NewL()
    {
    CIRClient* self = new( ELeave ) CIRClient( );
    CleanupStack::PushL( self );
    self->ConstructL( );
    CleanupStack::Pop();
    return self;
    }

    
// Destructor
CIRClient::~CIRClient()
    { 

    Cancel();
    if(iClient)
        {
        delete iClient;
        iClient = NULL;        
        }    
    
    iFileSession.Close();    
    delete iConnectObject;
    
    for (TInt index = 0; index < iFileArray.Count(); index++ )
    	{
    	if(iFileArray[index].SubSessionHandle())
    		{
    		iFileArray[index].Close();
    		}
    	}
    	
    iFileArray.Close();
    
    if ( iPutBufObject )
        {        
        delete iPutBufObject;
        iPutBufObject = NULL;
        }
    }

// -----------------------------------------------------------------------------
// CBTServiceClient::PutObjectL
// -----------------------------------------------------------------------------
//
void CIRClient::PutObjectL( )
    {
    FLOG(_L("[BTSU]\t CBTServiceClient::PutObjectL()"));

    if ( iPutBufObject )
        {        
        delete iPutBufObject;
        iPutBufObject = NULL;
        }

    // Create object
    //
    iPutBufObject = CObexBufObject::NewL(NULL);    
    
    RFile file;    
    file.Duplicate(iFileArray[iObjectIndex]);
    
    iBuffer = CBufFlat::NewL(1000);
    iBuffer ->ResizeL(1000);
    TObexRFileBackedBuffer bufferdetails(*iBuffer,file,CObexBufObject::ESingleBuffering);
    iPutBufObject->SetDataBufL(bufferdetails);
    
    
    TFileName filename;    
    file.Name(filename);
    
    TInt size;
    file.Size(size);
    iPutBufObject->SetLengthL(size);	
		
    iPutBufObject->SetNameL(filename);
    TTime time;
	if(file.Modified(time) == KErrNone)
		iPutBufObject->SetTimeL(time);
	
        
        

    // Send object
    //
    iClient->Put( *iPutBufObject, iStatus );    
	SetActive();
    iClientState = EIRCliPutting;

    FLOG(_L("[BTSU]\t CBTServiceClient::PutObjectL() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceClient::CloseClientConnection
// -----------------------------------------------------------------------------
//
void CIRClient::CloseClientConnection()
    {    

    iClient->Disconnect( iStatus );
   	SetActive();
    iClientState = EIRCliDisconnected;
    
    }

// -----------------------------------------------------------------------------
// CBTServiceClient::GetProgressStatus
// -----------------------------------------------------------------------------
//
TInt CIRClient::GetProgressStatus()
    {    

    TInt bytesSent = 0;
    if ( iPutBufObject )
        {
        bytesSent = iPutBufObject->BytesSent();
        }
        
    return iTotalBytesSent + bytesSent;
    }


// -----------------------------------------------------------------------------
// CBTServiceClient::DoCancel
// -----------------------------------------------------------------------------
//
void CIRClient::DoCancel()
    {
    FLOG(_L("[BTSU]\t CBTServiceClient::DoCancel()"));

    // Sending an error to Obex is the only way to cancel active requests
    //    
    if(iClient)
        {
        delete iClient;
        iClient = NULL;        
        }    
    

    FLOG(_L("[BTSU]\t CBTServiceClient::DoCancel() completed"));
    }

// -----------------------------------------------------------------------------
// CBTServiceClient::RunL
// -----------------------------------------------------------------------------
//
void CIRClient::RunL()
    {


    switch ( iClientState )
        {
        case EIRCliConnecting:
            {
               
            if ( iPutBufObject )
				{				
				delete iPutBufObject;
				iPutBufObject = NULL;
				}				
				iObjectIndex=0;		
				iDialog->CancelWaitDialogL();		
				iDialog->LaunchProgressDialogL( this, FileListsize(), 
            									R_IR_SENDING_DATA, KIRProgressInterval );             
				TRAPD(error,SendL());
				error=error;         	
			
			
            break;
            }

        case EIRCliPutting:
            {
                     
			if ( iPutBufObject )
				{				
				delete iPutBufObject;
				iPutBufObject = NULL;
				}		
		    if(iBuffer)
		        {
		        delete iBuffer;
		        iBuffer=NULL;
		        }
			iObjectIndex++;
			TRAPD(error,SendL());
			error=error;         
          
            break;
            }

        case EIRCliDisconnected:
            {            
            break;
            }       
        default:
           {
              
           break;
           }
        }

    FLOG(_L("[BTSU]\t CBTServiceClient::RunL() completed"));
    }
    
 // -----------------------------------------------------------------------------
// CBTServiceClient::DialogDismissed
// -----------------------------------------------------------------------------
//   
 void CIRClient::DialogDismissed( TInt aButtonId )
    {
    
    if( aButtonId == EAknSoftkeyCancel )
        {
        FLOG(_L("[BTSU]\t CBTServiceStarter::DialogDissmissed(), cancelled by user"));
        Cancel();

        // Cancelled by user, stop service
        //
        iClientState = EIRCliDisconnected;
        TRequestStatus* temp = &iStatus;
        User::RequestComplete( temp, KErrNone );        
        SetActive();
        }
    }


// -----------------------------------------------------------------------------
// CBTServiceClient::StartSendL
// -----------------------------------------------------------------------------
//     
 void CIRClient::StartSendL(const CAiwGenericParamList& aOutParamList )
    {
    
    if ( iDialog )
        {
        iDialog->CancelWaitDialogL();        
        }
    
    for( TInt index = 0; index < aOutParamList.Count(); index++ )
        {
        if ( aOutParamList[index].SemanticId() != EGenericParamFile )
            {
            FLOG(_L("[BTSS]\t CBTSSProvider::AddParametersL() wrong semantic Id: Leave"));
            User::Leave( KErrArgument );
            }
            // Try to add file as an image
            //          
       
    
        if(aOutParamList[index].Value().TypeId()== EVariantTypeFileHandle)
        	{        	        	
            AddFileHandleL( aOutParamList[index].Value().AsFileHandle() );           
        	
        	}
        else 
        	{            
            AddFileL( aOutParamList[index].Value().AsDes() );            
        	}
    
        }      
    
    }
    
// -----------------------------------------------------------------------------
// CBTServiceClient::SendL
// -----------------------------------------------------------------------------
//     
 void CIRClient::SendL()
    {
    if(iObjectIndex<iFileArray.Count())
        {        
        PutObjectL();
        }
     else
        {
        ShowNote();            
        iDialog->CancelProgressDialogL();
        iClient->Disconnect( iStatus );        
        SetActive();    
        iClientState=EIRCliDisconnected;        
        
        }
       
    }

// -----------------------------------------------------------------------------
// CBTServiceClient::AddFileHandleL
// -----------------------------------------------------------------------------
//      
    
    
 void CIRClient::AddFileHandleL(RFile aFile)
    {
    if(!aFile.SubSessionHandle())
        {
        User::Leave( KErrArgument );
        }
    RFile file;
    
    file.Duplicate(aFile);

    iFileArray.AppendL( file );
    
    }

// -----------------------------------------------------------------------------
// CBTServiceClient::AddFileL
// -----------------------------------------------------------------------------
//      
	    
 void CIRClient::AddFileL(const TDesC& aFilePath)
    {
    if ( &aFilePath == NULL || 
         aFilePath.Length() == 0 ||
         aFilePath.Length() > KMaxFileName )
        {
        User::Leave( KErrArgument );
        } 
    RFile file;
    User::LeaveIfError( file.Open( iFileSession, aFilePath, EFileShareReadersOnly) );
    AddFileHandleL(file);
    
    }
    
// -----------------------------------------------------------------------------
// CBTServiceClient::FileListsize
// -----------------------------------------------------------------------------
//      
 TInt CIRClient::FileListsize()
    {
    
    TInt totalFileSize = 0; 	    
    for ( TInt index = 0; index < iFileArray.Count(); index++ )
        {                
        TInt fileSize = 0;
        iFileArray[index].Size( fileSize );
        totalFileSize += fileSize;        
        }
    return totalFileSize;
    
    }

// -----------------------------------------------------------------------------
// CBTServiceClient::ShowNote
// -----------------------------------------------------------------------------
//      
void CIRClient::ShowNote()
    {
    if(iObjectIndex==iFileArray.Count() && iStatus==KErrNone)
        {
        TRAPD(error,TObexUtilsUiLayer::ShowInformationNoteL(R_IR_DATA_SENT));
        error=error;
        }
     else
     	{
     	TRAPD(error,TObexUtilsUiLayer::ShowInformationNoteL(R_IR_SENDING_FAILED));
     	error=error;
     	}
    }
//  End of File  
