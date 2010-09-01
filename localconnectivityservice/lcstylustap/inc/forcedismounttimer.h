/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef C_FORCEDISMOUNTTIMER_H
#define C_FORCEDISMOUNTTIMER_H

#include <e32base.h>

/**
 * Timer callback interface for force mount
 *
 * This class specifies the function to be called when a timeout occurs.
 * Used in conjunction with CTimeOutTimer class
 *  
 */    
class MTimerNotifier
    {
public:
    /**     
     * The function to be called when a timeout occurs.
     *     
     */
    virtual void TimerExpired() = 0;
    };

/**
 * Timer interface for force mount
 *
 * This class will notify an object after a specified timeout.
 * 
 */        
class CForceDismountTimer : public CTimer
    {
public:
    /**
     * Two-phased constructor.     
     * @param aTimeOutNotify object to notify of timeout event
     */
    static CForceDismountTimer* NewL( MTimerNotifier* aTimeOutNotify );

    /**
     * Two-phased constructor.     
     * @param aTimeOutNotify object to notify of timeout event
     */    
    static CForceDismountTimer* NewLC( MTimerNotifier* aTimeOutNotify);

    /**
    * Destructor
    */
    virtual ~CForceDismountTimer();

protected: 

    /**
     * From CTimer
     * Invoked when a timeout occurs
     *      
     */
    virtual void RunL();

private:
    /**
     * Constructor.     
     * @param aTimeOutNotify object to notify of timeout event
     */    
    CForceDismountTimer( MTimerNotifier* aTimeOutNotify );
    
    /**
     *  Default constructor.          
     */   
    void ConstructL();

private: // Member variables

    /**
     *  The observer for this objects events 
     *  Not own.
     */
    MTimerNotifier* iNotify;
    

    };
#endif // C_FORCEMOUNTTIMER_H
