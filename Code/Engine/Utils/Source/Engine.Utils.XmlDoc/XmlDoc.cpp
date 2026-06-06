// Copyright (c) Darrin Stewart. All rights reserved.
#include "Engine.Utils.XmlDoc/XmlDoc.h"
#include "XmlDocInternal.h"

void destroyXmlDoc(XmlDocHandle doc)
{
	delete static_cast<XmlDocument*>(doc);
}
