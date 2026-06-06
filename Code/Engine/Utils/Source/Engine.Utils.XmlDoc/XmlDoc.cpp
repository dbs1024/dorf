// Copyright (c) Darrin Stewart. All rights reserved.
#include "Engine.Utils.XmlDoc/XmlDoc.h"
#include "XmlDocInternal.h"

#include <cstring>
#include <new>

XmlResult createXmlDoc(XmlDocument** outDoc)
{
	*outDoc = new (std::nothrow) XmlDocument;
	if (!*outDoc)
		return XmlResult::InternalError;

	memset(*outDoc, 0, sizeof(XmlDocument));

	if (createFixedItemPool((*outDoc)->nodePool, sizeof(XmlNode), 65536) != FixedItemPoolResult::Success)
	{
		delete *outDoc;
		*outDoc = nullptr;
		return XmlResult::InternalError;
	}

	if (createFixedItemPool((*outDoc)->attributePool, sizeof(XmlAttribute), 65536) != FixedItemPoolResult::Success)
	{
		destroyFixedItemPool((*outDoc)->nodePool);
		delete *outDoc;
		*outDoc = nullptr;
		return XmlResult::InternalError;
	}

	constexpr int dataBufferSize = 4 * 1024 * 1024;
	(*outDoc)->dataStart = new (std::nothrow) char[dataBufferSize];
	if (!(*outDoc)->dataStart)
	{
		destroyFixedItemPool((*outDoc)->attributePool);
		destroyFixedItemPool((*outDoc)->nodePool);
		delete *outDoc;
		*outDoc = nullptr;
		return XmlResult::InternalError;
	}
	(*outDoc)->dataCurr = (*outDoc)->dataStart;
	(*outDoc)->dataEnd  = (*outDoc)->dataStart + dataBufferSize;

	return XmlResult::Ok;
}

void destroyXmlDoc(XmlDocument* doc)
{
	if (!doc)
		return;
	destroyFixedItemPool(doc->attributePool);
	destroyFixedItemPool(doc->nodePool);
	delete[] doc->dataStart;
	delete doc;
}
