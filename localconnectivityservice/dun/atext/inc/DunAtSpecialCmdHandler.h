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
* Description:  Special AT command handler
*
*/

#ifndef C_CDUNATSPECIALCMDHANDLER_H
#define C_CDUNATSPECIALCMDHANDLER_H

#include <e32base.h>
#include <badesca.h>

const TInt KInputBufLength  = (512 + 1);  // 512 chars for command + <CR>

/**
 *  Class for special AT command handler
 *
 *  @lib dunatext.lib
 *  @since S60 v5.0
 */
NONSHARABLE_CLASS( CDunAtSpecialCmdHandler ) : public CBase
    {

public:

    /**
     * Two-phased constructor.
     * @param None
     * @return Instance of self
     */
	static CDunAtSpecialCmdHandler* NewL();

    /**
    * Destructor.
    */
    ~CDunAtSpecialCmdHandler();
    
public:
    
    TBool IsCompleteSubCommand(TChar aCharacter);
    TBool IsCompleteSubCommand(TDesC8& aDes, TInt aStartIndex, TInt& aEndIndex);

private:

    CDunAtSpecialCmdHandler();

    void ConstructL();
    
    TBool IsDataReadyForComparison(TInt aLength);
    TInt MinimumLength();


private:  // data
    /**
     * Buffer for temporary AT command input
     */
    TBuf8<KInputBufLength> iBuffer;

    /**
     * Special commands for parsing
     */
    CDesC8Array *iSpecialCmds;    
    };

#endif  // C_CDUNATSPECIALCMDHANDLER_H
