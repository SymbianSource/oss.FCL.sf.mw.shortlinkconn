/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Converter class for converting AIW paramerer list to 
*                bt send parameter list
*
*/


#ifndef BTSSSENDLISTHANDLER_H
#define BTSSSENDLISTHANDLER_H

#include <e32base.h>
#include <AiwVariantType.hrh>
#include <AiwVariant.h>
#include <AiwGenericParam.h>

class CBTServiceParameterList;

/**
 *  BTSSend list handler
 *  Converts AIW list to bt sending object list 
 * 
 *  @since S60 v3.2
 */
class CBTSSSendListHandler : public CActive
    {
        
public:

    static CBTSSSendListHandler* NewL();
    static CBTSSSendListHandler* NewLC();    

    /**
    * Destructor.
    */
    virtual ~CBTSSSendListHandler();

    /**
     * ConvertList
     *
     * @since S60 v3.2
     * @param aOutParamList AIW parameter list
     * @param aList bt sending parameter list
     * @return error code
     */
     TInt ConvertList(const CAiwGenericParamList* aOutParamList, CBTServiceParameterList* aList  );

     
     
// from base class CActive
     
   /**
    * From CActive.
    * RunL
    *
    * @since S60 v3.2
    */
    void RunL();
     
    /**
     * From CActive.
     * DoCancel
     *
     * @since S60 v3.2
     */
    void DoCancel();


private:
    CBTSSSendListHandler();

    void ConstructL();

    /**
     * Add object 
     * 
     * @since S60 v3.2
     */
    void AddObject();
    
    /**
     * Add object 
     * 
     * @since S60 v3.2
     */
    void DoAddObjectL();
    
private: // data

    /**
     * BT sending parameter list
     * Not own.
     */
    CBTServiceParameterList* iList;

    /**
     * AIW parameter list
     * Not own.
     */
    const CAiwGenericParamList* iOutParamList; 
    
    /**
     * List index 
     */
    TInt iListIndex;
    
    /**
     * Sync waiter object 
     */
    CActiveSchedulerWait    iSyncWaiter;
    };

#endif // BTSSSENDLISTHANDLER_H
