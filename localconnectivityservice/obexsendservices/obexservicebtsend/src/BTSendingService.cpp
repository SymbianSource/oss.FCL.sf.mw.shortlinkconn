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
* Description:  Defines the DLL entry point and ECom initialization.
*                and a panic function used in BTSS.
*
*/


// INCLUDE FILES
#include "BTSSProvider.h"
#include "BTSendingServiceUIDS.hrh"
#include <e32std.h>
#include <implementationproxy.h>

// ============================= LOCAL FUNCTIONS ===============================
//

// -----------------------------------------------------------------------------
// ImplementationTable
// ECom init: Maps the interface UIDs to implementation factory functions.
// Returns: A table of implementation UIDs and their constructors.
// -----------------------------------------------------------------------------
//
const TImplementationProxy ImplementationTable[] =
    {
   	IMPLEMENTATION_PROXY_ENTRY(KBTSendingServiceMenuImplUid, CBTSSProvider::NewL),
   	IMPLEMENTATION_PROXY_ENTRY(KBTSendingServiceBaseImplUid, CBTSSProvider::NewL)
    };

// ========================== OTHER EXPORTED FUNCTIONS =========================



// -----------------------------------------------------------------------------
// ImplementationGroupProxy
// ECom init: Exported proxy for instantiation method resolution.
// Returns: Pointer to the proxy and number of implementations in it.
// -----------------------------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy( TInt& aTableCount )
    {
    aTableCount = sizeof( ImplementationTable ) / sizeof( TImplementationProxy );
    return ImplementationTable;
    }

//  End of File  
