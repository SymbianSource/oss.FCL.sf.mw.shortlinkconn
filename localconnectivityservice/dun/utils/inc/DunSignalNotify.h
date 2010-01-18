/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Monitors signal changes on network side and reports changes
*
*/


#ifndef C_CDUNSIGNALNOTIFY_H
#define C_CDUNSIGNALNOTIFY_H

#include "DunTransUtils.h"
#include "DunAtCmdHandler.h"

/**
 *  Class for monitoring line status and reporting changes
 *
 *  @lib dunutils.lib
 *  @since S60 v3.2
 */
NONSHARABLE_CLASS( CDunSignalNotify ) : public CActive
    {

public:

    /**
     * Two-phased constructor.
     * @param aUtility Pointer to common utility class
     * @return Instance of self
     */
	static CDunSignalNotify* NewL( MDunTransporterUtilityAux* aUtility );

    /**
    * Destructor.
    */
    virtual ~CDunSignalNotify();

    /**
     * Resets data to initial values
     *
     * @since S60 3.2
     * @return None
     */
    void ResetData();

    /**
     * Adds callback for line status change controlling
     * The callback will be called when line status change is detected in
     * endpoint
     *
     * @since S60 3.2
     * @param aCallback Callback to call when line status changes
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt AddCallback( MDunConnMon* aCallback );

    /**
     * Adds callback for endpoint readiness
     * The callback will be called when the endpoint is ready or not ready
     *
     * @since S60 5.0
     * @param aERCallback Callback to call when writes can/can't be done
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt AddEndpointReadyCallback( MDunEndpointReady* aERCallback );

    /**
     * Sets media to use for this endpoint monitor (network side)
     *
     * @since S60 3.2
     * @param aComm RComm pointer to use as the endpoint
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt SetMedia( RComm* aComm );

    /**
     * Issues request to start monitoring the endpoint for line status change
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt IssueRequest();

    /**
     * Stops monitoring the endpoint for line status change
     *
     * @since S60 3.2
     * @return Symbian error code on error, KErrNone otherwise
     */
    TInt Stop();

private:

    CDunSignalNotify( MDunTransporterUtilityAux* aUtility );

    void ConstructL();

    /**
     * Initializes this class
     *
     * @since S60 3.2
     * @return None
     */
    void Initialize();

    /**
     * Manages signal changes
     *
     * @since S60 3.2
     * @return None
     */
    void ManageSignalChange();

    /**
     * Reports signal change
     *
     * @since S60 3.2
     * @param aSetMask Set the handshaking lines in the mask
     * @param aClearMask Clear the handshaking lines in the mask.
     * @return None
     */
    void ReportSignalChange( TUint aSetMask, TUint aClearMask );

    /**
     * Reports endpoint ready or not ready
     *
     * @since S60 5.0
     * @param aReady ETrue if endpoint ready, EFalse otherwise
     * @return None
     */
    void ReportEndpointReady( TBool aReady );

// from base class CActive

    /*
     * From CActive.
     * Gets called when line status changes
     *
     * @since S60 3.2
     * @return None
     */
    void RunL();

    /**
     * From CActive.
     * Gets called on cancel
     *
     * @since S60 3.2
     * @return None
     */
    void DoCancel();

private:  // data

    /**
     * Callback(s) to call when notification(s) via MDunConnMon to be made
     * Normally contains only one callback
     */
    RPointerArray<MDunConnMon> iCallbacks;

    /**
     * Callback(s) to call when notification(s) via MDunEndpointReady to be made
     * Normally contains only one callback for upstream
     */
    RPointerArray<MDunEndpointReady> iERCallbacks;

    /**
     * Pointer to common utility class
     * Not own.
     */
    MDunTransporterUtilityAux* iUtility;

    /**
     * Current state of signal monitoring: active or inactive
     */
    TDunState iSignalNotifyState;

    /**
     * Signals to listen with RComm::NotifySignalChange()
     */
    TUint iListenSignals;

    /**
     * Signals set when RComm::NotifySignalChange() request completes
     */
    TUint iSignals;

    /**
     * RComm object of network side
     * Not own.
     */
    RComm* iNetwork;

    };

#endif  // C_CDUNSIGNALNOTIFY_H
