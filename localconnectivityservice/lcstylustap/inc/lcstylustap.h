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
* Description:  Header file for Stylus Tap indicator
*
*/


#ifndef C_LCSTYLUSTAP_H
#define C_LCSTYLUSTAP_H

#include <AknIndicatorPlugin.h>


class CLcStylusTapDismount;
/**
 * Implements stylus tap actions for the indicator provided by
 * UID
 * @lib 
 * @since S60 v5.0
 */
        
NONSHARABLE_CLASS( CLcStylusTap ) :  public CAknIndicatorPlugin
    {

public:

    /**
     * Two-phase constructor
     * @since S60 5.0
     * @param  none
     */
    static CLcStylusTap* NewL( );
    
    /**
     * Destructor
     */
    virtual ~CLcStylusTap();

public: // CAknIndicatorPlugin

    /**
     * HandleIndicatorTapL is called when user tap on the UI 
     * @since S60 5.0
     * @param  aUid, implementation Uid 
     */
    void HandleIndicatorTapL( const TInt aUid );
    /**
     * TextL, the text that should be shown in the tap area
     * @since S60 5.0
     * @param  aUid, implementation Uid
     * @param  aTextType,  text type linked or no link
     */
    HBufC* TextL( const TInt aUid, TInt& aTextType );

private:

    /**
     * C++ default constructor
     * @since S60 5.0
     * @param  none     
     */
    CLcStylusTap( );

    /**
     * Symbian 2nd-phase constructor
     * @since S60 5.0
     * @param  none     
     */
    void ConstructL();
    /**
     * Create the setting application
     * @since S60 5.0
     * @param  aProcessName, process name whose setting view will be launched
     * @param  aUidType TUidType of the desired process
     */
    void CreateDesiredViewL(const TDesC & aProcessName,const TUidType & aUidType) const;
    
    
    /**
     * Ejec usb memory
     */
    void EjectUSBMemL();

private: // data
    /**
     * Dismount manageer
     * Own.
     */
    CLcStylusTapDismount*   iDismountManager;
    };


#endif      // __LCSTYLUSTAP_H__
