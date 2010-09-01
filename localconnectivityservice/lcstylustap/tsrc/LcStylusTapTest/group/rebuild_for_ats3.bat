@rem
@rem Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
@rem All rights reserved.
@rem This component and the accompanying materials are made available
@rem under the terms of "Eclipse Public License v1.0"
@rem which accompanies this distribution, and is available
@rem at the URL "http://www.eclipse.org/legal/epl-v10.html".
@rem
@rem Initial Contributors:
@rem Nokia Corporation - initial contribution.
@rem
@rem Contributors:
@rem
@rem Description: 
@rem

echo Kompilacja na armv5 udeb

del LcStylusTapTest.sisx
rd /S /Q \epoc32\BUILD
call bldmake bldfiles
call abld test reallyclean armv5 udeb
call abld test build armv5 udeb
call abld test freeze armv5
call make_and_sign_sis.bat LcStylusTapTest

call pause