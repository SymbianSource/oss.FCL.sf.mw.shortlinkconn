/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This class handles services management requests.
*
*/



// INCLUDE FILES
#include "SrcsServiceManager.h"
#include "debug.h"


// CONSTANTS

// The granularity of the array used to hold BT, IrDA and USB connnection objects
static const TInt KConnectionArrayGranularity = 4;

// ================= MEMBER FUNCTIONS =======================


// ---------------------------------------------------------
// C++ default constructor can NOT contain any code, that
// might leave.
// ---------------------------------------------------------
//
CSrcsServiceManager::CSrcsServiceManager():CActive(CActive::EPriorityStandard)
    {
    CActiveScheduler::Add(this);    
    }
    
// ---------------------------------------------------------
// Destructor
// ---------------------------------------------------------
//
CSrcsServiceManager::~CSrcsServiceManager()
    {       
    Cancel();    
    if ( iBTConnectionArray )
        {
        // Cleanup the array
        iBTConnectionArray->ResetAndDestroy();
        }
    delete iBTConnectionArray;

    if ( iUSBConnectionArray )
        {
        // Cleanup the array
        iUSBConnectionArray->ResetAndDestroy();        
        }
    delete iUSBConnectionArray;


    if ( iIrDAConnectionArray )
        {
        // Cleanup the array
        iIrDAConnectionArray->ResetAndDestroy();
        }
    delete iIrDAConnectionArray;

	REComSession::FinalClose();
    }

// ---------------------------------------------------------
// NewL
// ---------------------------------------------------------
//
CSrcsServiceManager* CSrcsServiceManager::NewL()
    {
    FLOG(_L("[SRCS]\tserver\tCSrcsServiceManager: NewL"));
    CSrcsServiceManager* self = new (ELeave) CSrcsServiceManager();
	CleanupStack::PushL(self);
    self->ConstructL();
	CleanupStack::Pop();
    return self;
    }

// ---------------------------------------------------------
// ConstructL
// ---------------------------------------------------------
//
void CSrcsServiceManager::ConstructL()
    {
    iBTConnectionArray = new(ELeave) CArrayPtrFlat<CSrcsTransport>(KConnectionArrayGranularity);
    iUSBConnectionArray = new(ELeave) CArrayPtrFlat<CSrcsTransport>(KConnectionArrayGranularity);
	iIrDAConnectionArray = new(ELeave) CArrayPtrFlat<CSrcsTransport>(KConnectionArrayGranularity);
    }

// ---------------------------------------------------------
// ManagerServicesL
// Method to manage service controllers on all supported transports.
// ---------------------------------------------------------
//
TInt CSrcsServiceManager::ManageServices( TSrcsTransport aTransport, TBool aState, 
                                            MObexSMRequestObserver* aObserver, 
                                            const RMessage2& aMessage)
    {
    FLOG(_L("[SRCS]\tserver\tCSrcsServiceManager: ManageServices"));
    if ( !IsActive() ) 
        { 
        iStatus=KRequestPending; 
        DoManageServices( aTransport,aState, aObserver, aMessage );
        SetActive();
        FLOG(_L("[SRCS]\tserver\tCSrcsServiceManager: ManageServices KErrNone"));
        return KErrNone; 
        } 
    else 
        { 
        FLOG(_L("[SRCS]\tserver\tCSrcsServiceManager: ManageServices KErrServerBusy"));
        return KErrServerBusy; 
        }
    }

// ---------------------------------------------------------
// DoManageServices
// Method to manage service controllers on all supported transports.
// ---------------------------------------------------------
//    
void CSrcsServiceManager::DoManageServices(TSrcsTransport aTransport, TBool aState, MObexSMRequestObserver* aObserver, 
                                            const RMessage2& aMessage)
    {
    FLOG(_L("[SRCS]\tserver\tCSrcsServiceManager: DoManageServices"));
    iObserver=aObserver;
    iMessage=aMessage;    
    TRAPD(error,RealDoManageServiceL(aTransport,aState));    
    if (error != KErrNone)
        {
        iErrorState=error;    
        }
    TRequestStatus* temp = &iStatus;
    User::RequestComplete( temp, iErrorState );
    FLOG(_L("[SRCS]\tserver\tCSrcsServiceManager: DoManageServices exit"));
    }

// ---------------------------------------------------------
// RunL
// Notifies request completion
// ---------------------------------------------------------
//    
void CSrcsServiceManager::RunL()
    {
    FLOG(_L("[SRCS]\tserver\tCSrcsServiceManager: RunL"));
    iObserver->RequestCompleted(iMessage,iStatus.Int());
    FLOG(_L("[SRCS]\tserver\tCSrcsServiceManager: RunL exit"));
    }
// ---------------------------------------------------------
// RunErrorL
// ---------------------------------------------------------
//        
void CSrcsServiceManager::RunError()
    {
    }
// ---------------------------------------------------------
// DoCancel
// ---------------------------------------------------------
//            
void CSrcsServiceManager::DoCancel()
    {
    FLOG(_L("[SRCS]\tserver\tCSrcsServiceManager: DoCancel"));
    }
// ---------------------------------------------------------
// RealDoManageServiceL
// Method to manage service controllers on all supported transports.
// ---------------------------------------------------------
//                
void CSrcsServiceManager::RealDoManageServiceL(TSrcsTransport aTransport, TBool aState)
    {    
    FLOG(_L("[SRCS]\tserver\tCSrcsServiceManager: RealDoManageServiceL"));
    CArrayPtr<CSrcsTransport>* connectionArray=NULL;
    TPtrC8 transportName;

    switch(aTransport)
        {
    case ESrcsTransportBT:
	    FLOG(_L("[SRCS]\tserver\tCSrcsServiceManager: ManageServicesL(Bluetooth)"));
        transportName.Set(KSrcsTransportBT);
        connectionArray = iBTConnectionArray;
        break;
    case ESrcsTransportUSB:
	    FLOG(_L("[SRCS]\tserver\tCSrcsServiceManager: ManageServicesL(USB)"));
        transportName.Set(KSrcsTransportUSB);
        connectionArray = iUSBConnectionArray;        
        break;
    case ESrcsTransportIrDA:
	    FLOG(_L("[SRCS]\tserver\tCSrcsServiceManager: ManageServicesL(IrDA)"));
        transportName.Set(KSrcsTransportIrDA);
        connectionArray = iIrDAConnectionArray;
        break;
    default:
        FTRACE(FPrint(_L("[SRCS]\tserver\tCSrcsServiceManager: ManageServicesL. Transport not supported.")));
        iErrorState = KErrNotSupported;              
        }
    // We start and stop services by aState value
    if ( aState ) // trun on service
        {
	    FLOG(_L("[SRCS]\tserver\tCSrcsServiceManager: ManageServicesL(Turn ON)"));
        // We do not re-start services if they have been started        
        if ( connectionArray && !(connectionArray->Count()) )
            {   			
			//Declare array of service controllers
            RImplInfoPtrArray infoArrayServiceController;                        
                
			//Declare array of SRCS transport plugins
            RImplInfoPtrArray infoArrayTranport;
            CleanupClosePushL(infoArrayTranport);		
            
            CleanupClosePushL(infoArrayServiceController);

            //List all SRCS transport plugin implementations
            FLOG(_L("[SRCS]\tserver\tCSrcsServiceManager: ManageServicesL about to list Transport Impl"));
            CSrcsTransport::ListImplementationsL(transportName,infoArrayTranport);

			//Found SRCS transport plugin. Then start to enumerate service controller and make connections.
            if(infoArrayTranport.Count())
                {
				//There should be only one transport plugin of each type. Others are just ignored.
                if(infoArrayTranport.Count() != 1) 
                    {
                    FTRACE(FPrint(_L("[SRCS]\tserver\tCSrcsServiceManager: ManageServicesL. Warning: Found %d transport implementations." ), infoArrayTranport.Count()));
                    }

                FTRACE(FPrint(_L("[SRCS]\tserver\tCSrcsServiceManager: ManageServicesL. Using Transport ImplementationUid %x"), infoArrayTranport[0]->ImplementationUid()));

				//enumerate service controllers
                CSrcsInterface::ListImplementationsL(transportName,infoArrayServiceController);

                // Loop through each found service controller, 
                // create SRCS transport connection for each found service controller
				// and instantiate the service controller.
                CSrcsTransport *cm;

                for (TInt i=0; i< infoArrayServiceController.Count(); i++)
                    {
                    // TRAP is needed because of OOM situations.
                    // Otherwise whole server is leaving and panicing.
                    FTRACE(FPrint(_L("[SRCS]\tserver\tCSrcsServiceManager: ManageServicesL. Found Service Controller ImplementationUid %x"), infoArrayServiceController[i]->ImplementationUid()));                  
                    
                    TRAPD( error, cm = CSrcsTransport::NewL(infoArrayTranport[0]->ImplementationUid(), infoArrayServiceController[i] ));
                    if ( error != KErrNone )
                        {
                        // Error when creating service controller (e.g. no memory). Cleanup and zero.
                        FTRACE(FPrint(_L("[SRCS]\tserver\tCSrcsServiceManager: ManageServicesL. Create implementation failed with error code %d"), error));
                        }
                    else
                        {
                        // Add this connection to the list
                        connectionArray->AppendL(cm);
                        FTRACE(FPrint(_L("[SRCS]\tserver\tCSrcsServiceManager: ManageServicesL: Implementation created successfully.")));
                        }
                    }
                }
            else
                {
                FTRACE(FPrint(_L("[SRCS]\tserver\tCSrcsServiceManager: ManageServicesL. Transport implementation not found.")));
                }

            // Clean up            
           infoArrayTranport.ResetAndDestroy();                 
           infoArrayServiceController.ResetAndDestroy();                
           CleanupStack::PopAndDestroy(2); //infoArrayServiceController
           
            }
        }
    else // turn off service
        {
	    FLOG(_L("[SRCS]\tserver\tCSrcsServiceManager: ManageServicesL(Turn OFF)"));          	        
        connectionArray->ResetAndDestroy();                 
        }        
    FLOG(_L("[SRCS]\tserver\tCSrcsServiceManager: RealDoManageServiceL exit"));
    }    
// End of file
