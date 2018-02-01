/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ua_server_internal.h"
#include "ua_services.h"

static UA_StatusCode
fillReferenceDescription(UA_NodeStore *ns, const UA_Node *curr, UA_ReferenceNode *ref,
                         UA_UInt32 mask, UA_ReferenceDescription *descr) {
    UA_ReferenceDescription_init(descr);
    UA_StatusCode retval = UA_NodeId_copy(&curr->nodeId, &descr->nodeId.nodeId);
    if(mask & UA_BROWSERESULTMASK_REFERENCETYPEID)
        retval |= UA_NodeId_copy(&ref->referenceTypeId, &descr->referenceTypeId);
    if(mask & UA_BROWSERESULTMASK_ISFORWARD)
        descr->isForward = !ref->isInverse;
    if(mask & UA_BROWSERESULTMASK_NODECLASS)
        retval |= UA_NodeClass_copy(&curr->nodeClass, &descr->nodeClass);
    if(mask & UA_BROWSERESULTMASK_BROWSENAME)
        retval |= UA_QualifiedName_copy(&curr->browseName, &descr->browseName);
    if(mask & UA_BROWSERESULTMASK_DISPLAYNAME)
        retval |= UA_LocalizedText_copy(&curr->displayName, &descr->displayName);
    if(mask & UA_BROWSERESULTMASK_TYPEDEFINITION){
        if(curr->nodeClass == UA_NODECLASS_OBJECT || curr->nodeClass == UA_NODECLASS_VARIABLE) {
            for(size_t i = 0; i < curr->referencesSize; ++i) {
                UA_ReferenceNode *refnode = &curr->references[i];
                if(refnode->referenceTypeId.identifier.numeric == UA_NS0ID_HASTYPEDEFINITION) {
                    retval |= UA_ExpandedNodeId_copy(&refnode->targetId, &descr->typeDefinition);
                    break;
                }
            }
        }
    }
    return retval;
}

#ifdef UA_ENABLE_EXTERNAL_NAMESPACES
static const UA_Node *
returnRelevantNodeExternal(UA_ExternalNodeStore *ens, const UA_BrowseDescription *descr,
                           const UA_ReferenceNode *reference) {
    /* prepare a read request in the external nodestore */
    UA_ReadValueId *readValueIds = UA_Array_new(5,&UA_TYPES[UA_TYPES_READVALUEID]);
    UA_UInt32 *indices = UA_Array_new(5,&UA_TYPES[UA_TYPES_UINT32]);
    UA_UInt32 indicesSize = 5;
    UA_DataValue *readNodesResults = UA_Array_new(5,&UA_TYPES[UA_TYPES_DATAVALUE]);
    UA_DiagnosticInfo *diagnosticInfos = UA_Array_new(5,&UA_TYPES[UA_TYPES_DIAGNOSTICINFO]);
    for(UA_UInt32 i = 0; i < 5; ++i) {
        readValueIds[i].nodeId = reference->targetId.nodeId;
        indices[i] = i;
    }
    readValueIds[0].attributeId = UA_ATTRIBUTEID_NODECLASS;
    readValueIds[1].attributeId = UA_ATTRIBUTEID_BROWSENAME;
    readValueIds[2].attributeId = UA_ATTRIBUTEID_DISPLAYNAME;
    readValueIds[3].attributeId = UA_ATTRIBUTEID_DESCRIPTION;
    readValueIds[4].attributeId = UA_ATTRIBUTEID_WRITEMASK;

    ens->readNodes(ens->ensHandle, NULL, readValueIds, indices,
                   indicesSize, readNodesResults, false, diagnosticInfos);

    /* create and fill a dummy nodeStructure */
    UA_Node *node = (UA_Node*) UA_NodeStore_newObjectNode();
    UA_NodeId_copy(&(reference->targetId.nodeId), &(node->nodeId));
    if(readNodesResults[0].status == UA_STATUSCODE_GOOD)
        UA_NodeClass_copy((UA_NodeClass*)readNodesResults[0].value.data, &(node->nodeClass));
    if(readNodesResults[1].status == UA_STATUSCODE_GOOD)
        UA_QualifiedName_copy((UA_QualifiedName*)readNodesResults[1].value.data, &(node->browseName));
    if(readNodesResults[2].status == UA_STATUSCODE_GOOD)
        UA_LocalizedText_copy((UA_LocalizedText*)readNodesResults[2].value.data, &(node->displayName));
    if(readNodesResults[3].status == UA_STATUSCODE_GOOD)
        UA_LocalizedText_copy((UA_LocalizedText*)readNodesResults[3].value.data, &(node->description));
    if(readNodesResults[4].status == UA_STATUSCODE_GOOD)
        UA_UInt32_copy((UA_UInt32*)readNodesResults[4].value.data, &(node->writeMask));

    UA_ReferenceNode **references = &node->references;
    UA_UInt32 *referencesSize = (UA_UInt32*)&node->referencesSize;

    ens->getOneWayReferences (ens->ensHandle, &node->nodeId, referencesSize, references);

    UA_Array_delete(readValueIds,5, &UA_TYPES[UA_TYPES_READVALUEID]);
    UA_Array_delete(indices,5, &UA_TYPES[UA_TYPES_UINT32]);
    UA_Array_delete(readNodesResults,5, &UA_TYPES[UA_TYPES_DATAVALUE]);
    UA_Array_delete(diagnosticInfos,5, &UA_TYPES[UA_TYPES_DIAGNOSTICINFO]);
    if(node && descr->nodeClassMask != 0 && (node->nodeClass & descr->nodeClassMask) == 0) {
        UA_NodeStore_deleteNode(node);
        return NULL;
    }
    return node;
}
#endif

/* Tests if the node is relevant to the browse request and shall be returned. If
   so, it is retrieved from the Nodestore. If not, null is returned. */
static const UA_Node *
returnRelevantNode(UA_Server *server, const UA_BrowseDescription *descr,
                   const UA_ReferenceNode *reference, const UA_NodeId *relevant, size_t relevant_count) {
    /* reference in the right direction? */
    if(reference->isInverse && descr->browseDirection == UA_BROWSEDIRECTION_FORWARD)
        return NULL;
    if(!reference->isInverse && descr->browseDirection == UA_BROWSEDIRECTION_INVERSE)
        return NULL;

    /* Is the reference part of the hierarchy of references we look for? For
     * this, relevant != NULL.*/
    if(relevant) {
        UA_Boolean is_relevant = false;
        for(size_t i = 0; i < relevant_count; ++i) {
            if(UA_NodeId_equal(&reference->referenceTypeId, &relevant[i])) {
                is_relevant = true;
                break;
            }
        }
        if(!is_relevant)
            return NULL;
    }

#ifdef UA_ENABLE_EXTERNAL_NAMESPACES
    /* return the node from an external namespace*/
    for(size_t nsIndex = 0; nsIndex < server->externalNamespacesSize; ++nsIndex) {
        if(reference->targetId.nodeId.namespaceIndex != server->externalNamespaces[nsIndex].index)
            continue;
        return returnRelevantNodeExternal(&server->externalNamespaces[nsIndex].externalNodeStore,
                                          descr, reference);
    }
#endif

    /* return from the internal nodestore */
    const UA_Node *node = UA_NodeStore_get(server->nodestore, &reference->targetId.nodeId);
    if(node && descr->nodeClassMask != 0 && (node->nodeClass & descr->nodeClassMask) == 0)
        return NULL;
    return node;
}

static void removeCp(struct ContinuationPointEntry *cp, UA_Session* session) {
    LIST_REMOVE(cp, pointers);
    UA_ByteString_deleteMembers(&cp->identifier);
    UA_BrowseDescription_deleteMembers(&cp->browseDescription);
    UA_free(cp);
    ++session->availableContinuationPoints;
}

static UA_Boolean
browseRelevantReferences(UA_Server *server, UA_BrowseResult *result, const UA_NodeId *relevant_refs,
                         size_t relevant_refs_size, const UA_BrowseDescription *descr,
                         struct ContinuationPointEntry *cp) {
    /* Get the node */
    const UA_Node *node = UA_NodeStore_get(server->nodestore, &descr->nodeId);
    if(!node) {
        result->statusCode = UA_STATUSCODE_BADNODEIDUNKNOWN;
        return true;;
    }

    /* If the node has no references, just return */
    if(node->referencesSize == 0) {
        result->referencesSize = 0;
        return true;;
    }

    /* How many references can we return at most? */
    size_t maxrefs = cp->maxReferences;
    if(maxrefs == 0)
        maxrefs = node->referencesSize;
    else if(maxrefs > node->referencesSize)
        maxrefs = node->referencesSize;

    UA_assert(node->referencesSize > 0);
    UA_assert(maxrefs > 0);
    UA_assert(maxrefs <= node->referencesSize);

    /* Allocate the results array */
    result->references =
        (UA_ReferenceDescription*)UA_Array_new(maxrefs, &UA_TYPES[UA_TYPES_REFERENCEDESCRIPTION]);
    if(!result->references) {
        result->statusCode = UA_STATUSCODE_BADOUTOFMEMORY;
        return false;
    }

    /* Loop over the node's references */
    size_t skipped = 0;
    size_t referencesCount = 0; /* How many references did we copy into the results array */
    size_t i = 0; /* Count the references we looked at */
    for(; i < node->referencesSize && referencesCount < maxrefs; ++i) {
        const UA_Node *current = returnRelevantNode(server, descr, &node->references[i],
                                                    relevant_refs, relevant_refs_size);
        if(!current)
            continue;

        if(skipped < cp->continuationIndex) {
            ++skipped;
            continue;
        }

        result->statusCode = fillReferenceDescription(server->nodestore, current,
                                                      &node->references[i], descr->resultMask,
                                                      &result->references[referencesCount]);
        if(result->statusCode != UA_STATUSCODE_GOOD)
            break;

        ++referencesCount;
    }
    result->referencesSize = referencesCount;

    /* No relevant references, return array of length zero */
    if(referencesCount == 0) {
        UA_free(result->references);
        result->references = (UA_ReferenceDescription *)UA_EMPTY_ARRAY_SENTINEL;
        result->referencesSize = 0;
    }

    /* Clean up if an error occured */
    if(result->statusCode != UA_STATUSCODE_GOOD) {
        UA_Array_delete(result->references, result->referencesSize,
                        &UA_TYPES[UA_TYPES_REFERENCEDESCRIPTION]);
        result->references = NULL;
        result->referencesSize = 0;
        return false;
    }

    /* Are we done with the node? */
    return (i == node->referencesSize);
}

/* Results for a single browsedescription. This is the inner loop for both
 * Browse and BrowseNext
 *
 * @param session Session to save continuationpoints
 * @param ns The nodstore where the to-be-browsed node can be found
 * @param cp If cp is not null, we continue from here If cp is null, we can add
 *           a new continuation point if possible and necessary.
 * @param descr If no cp is set, we take the browsedescription from there
 * @param maxrefs The maximum number of references the client has requested. If 0,
 *                all matching references are returned at once.
 * @param result The entry in the request */
void
Service_Browse_single(UA_Server *server, UA_Session *session,
                      struct ContinuationPointEntry *cp, const UA_BrowseDescription *descr,
                      UA_UInt32 maxrefs, UA_BrowseResult *result) {
    struct ContinuationPointEntry *internal_cp = cp;
    if(!internal_cp) {
        /* If there is no continuation point, stack-allocate one. It gets copied
         * on the heap when this is required at a later point. */
        internal_cp = (struct ContinuationPointEntry *)UA_alloca(sizeof(struct ContinuationPointEntry));
        memset(internal_cp, 0, sizeof(struct ContinuationPointEntry));
        internal_cp->maxReferences = maxrefs;
    } else {
        /* Set the browsedescription if a cp is given */
        descr = &cp->browseDescription;
    }

    /* Is the browsedirection valid? */
    if(descr->browseDirection != UA_BROWSEDIRECTION_BOTH &&
       descr->browseDirection != UA_BROWSEDIRECTION_FORWARD &&
       descr->browseDirection != UA_BROWSEDIRECTION_INVERSE) {
        result->statusCode = UA_STATUSCODE_BADBROWSEDIRECTIONINVALID;
        return;
    }

    /* Get the references that match the browsedescription. reftypes == NULL
     * indicates that all references shall be returned. */
    size_t reftypesSize = 0;
    UA_NodeId *reftypes = NULL;
    UA_Boolean all_refs = UA_NodeId_isNull(&descr->referenceTypeId);
    if(!all_refs) {
        const UA_Node *rootRef = UA_NodeStore_get(server->nodestore, &descr->referenceTypeId);
        if(!rootRef || rootRef->nodeClass != UA_NODECLASS_REFERENCETYPE) {
            result->statusCode = UA_STATUSCODE_BADREFERENCETYPEIDINVALID;
            return;
        }
        if(descr->includeSubtypes) {
            result->statusCode = getTypeHierarchy(server->nodestore, rootRef, false,
                                                  &reftypes, &reftypesSize);
            if(result->statusCode != UA_STATUSCODE_GOOD)
                return;
        } else {
            reftypes = (UA_NodeId*)(uintptr_t)&descr->referenceTypeId;
            reftypesSize = 1;
        }
    }

    /* Browse with the relevant references */
    UA_Boolean done = browseRelevantReferences(server, result, reftypes, reftypesSize, descr, internal_cp);

    /* Clean up the array of relevant references */
    if(!all_refs && descr->includeSubtypes)
        UA_Array_delete(reftypes, reftypesSize, &UA_TYPES[UA_TYPES_NODEID]);

    /* Exit early if an error occured */
    if(result->statusCode != UA_STATUSCODE_GOOD)
        return;

    /* Update the continuationIndex, how many results did we deliver so far for
     * the BrowseDescription? */
    internal_cp->continuationIndex += (UA_UInt32)result->referencesSize;

    /* A continuation point exists already */
    if(cp) {
        if(done)
            removeCp(cp, session); /* All done, remove a finished continuationPoint */
         else
             UA_ByteString_copy(&cp->identifier, &result->continuationPoint); /* Return the cp identifier */
        return;
    }

    /* Create a new continuation point */
    if(!done) {
        if(session->availableContinuationPoints <= 0 ||
           !(cp = (struct ContinuationPointEntry *)UA_malloc(sizeof(struct ContinuationPointEntry)))) {
            result->statusCode = UA_STATUSCODE_BADNOCONTINUATIONPOINTS;
            return;
        }
        UA_BrowseDescription_copy(descr, &cp->browseDescription);
        cp->continuationIndex = internal_cp->continuationIndex;
        cp->maxReferences = internal_cp->maxReferences;

        /* Create a random bytestring via a Guid */
        UA_Guid *ident = UA_Guid_new();
        *ident = UA_Guid_random();
        cp->identifier.data = (UA_Byte*)ident;
        cp->identifier.length = sizeof(UA_Guid);

        /* Return the cp identifier */
        UA_ByteString_copy(&cp->identifier, &result->continuationPoint);

        /* Attach the cp to the session */
        LIST_INSERT_HEAD(&session->continuationPoints, cp, pointers);
        --session->availableContinuationPoints;
    }
}

void Service_Browse(UA_Server *server, UA_Session *session, const UA_BrowseRequest *request,
                    UA_BrowseResponse *response) {
    UA_LOG_DEBUG_SESSION(server->config.logger, session, "Processing BrowseRequest");
    if(!UA_NodeId_isNull(&request->view.viewId)) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADVIEWIDUNKNOWN;
        return;
    }

    if(request->nodesToBrowseSize <= 0) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADNOTHINGTODO;
        return;
    }

    size_t size = request->nodesToBrowseSize;
    response->results = (UA_BrowseResult *)UA_Array_new(size, &UA_TYPES[UA_TYPES_BROWSERESULT]);
    if(!response->results) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;
        return;
    }
    response->resultsSize = size;

#ifdef UA_ENABLE_EXTERNAL_NAMESPACES
#ifdef NO_ALLOCA
    UA_Boolean isExternal[size];
    UA_UInt32 indices[size];
#else
    UA_Boolean *isExternal = UA_alloca(sizeof(UA_Boolean) * size);
    UA_UInt32 *indices = UA_alloca(sizeof(UA_UInt32) * size);
#endif /*NO_ALLOCA */
    memset(isExternal, false, sizeof(UA_Boolean) * size);
    for(size_t j = 0; j < server->externalNamespacesSize; ++j) {
        size_t indexSize = 0;
        for(size_t i = 0; i < size; ++i) {
            if(request->nodesToBrowse[i].nodeId.namespaceIndex != server->externalNamespaces[j].index)
                continue;
            isExternal[i] = true;
            indices[indexSize] = (UA_UInt32)i;
            ++indexSize;
        }
        if(indexSize == 0)
            continue;
        UA_ExternalNodeStore *ens = &server->externalNamespaces[j].externalNodeStore;
        ens->browseNodes(ens->ensHandle, &request->requestHeader, request->nodesToBrowse, indices,
                         (UA_UInt32)indexSize, request->requestedMaxReferencesPerNode,
                         response->results, response->diagnosticInfos);
    }
#endif

    for(size_t i = 0; i < size; ++i) {
#ifdef UA_ENABLE_EXTERNAL_NAMESPACES
        if(!isExternal[i])
#endif
            Service_Browse_single(server, session, NULL, &request->nodesToBrowse[i],
                                  request->requestedMaxReferencesPerNode, &response->results[i]);
    }
}

UA_BrowseResult
UA_Server_browse(UA_Server *server, UA_UInt32 maxrefs, const UA_BrowseDescription *descr) {
    UA_BrowseResult result;
    UA_BrowseResult_init(&result);
    UA_RCU_LOCK();
    Service_Browse_single(server, &adminSession, NULL, descr, maxrefs, &result);
    UA_RCU_UNLOCK();
    return result;
}

static void
UA_Server_browseNext_single(UA_Server *server, UA_Session *session, UA_Boolean releaseContinuationPoint,
                            const UA_ByteString *continuationPoint, UA_BrowseResult *result) {
    result->statusCode = UA_STATUSCODE_BADCONTINUATIONPOINTINVALID;
    struct ContinuationPointEntry *cp, *temp;
    LIST_FOREACH_SAFE(cp, &session->continuationPoints, pointers, temp) {
        if(UA_ByteString_equal(&cp->identifier, continuationPoint)) {
            result->statusCode = UA_STATUSCODE_GOOD;
            if(!releaseContinuationPoint)
                Service_Browse_single(server, session, cp, NULL, 0, result);
            else
                removeCp(cp, session);
            break;
        }
    }
}

void Service_BrowseNext(UA_Server *server, UA_Session *session, const UA_BrowseNextRequest *request,
                        UA_BrowseNextResponse *response) {
    UA_LOG_DEBUG_SESSION(server->config.logger, session, "Processing BrowseNextRequest");
    if(request->continuationPointsSize <= 0) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADNOTHINGTODO;
        return;
    }
    size_t size = request->continuationPointsSize;
    response->results = (UA_BrowseResult *)UA_Array_new(size, &UA_TYPES[UA_TYPES_BROWSERESULT]);
    if(!response->results) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;
        return;
    }

    response->resultsSize = size;
    for(size_t i = 0; i < size; ++i)
        UA_Server_browseNext_single(server, session, request->releaseContinuationPoints,
                                    &request->continuationPoints[i], &response->results[i]);
}

UA_BrowseResult
UA_Server_browseNext(UA_Server *server, UA_Boolean releaseContinuationPoint,
                     const UA_ByteString *continuationPoint) {
    UA_BrowseResult result;
    UA_BrowseResult_init(&result);
    UA_RCU_LOCK();
    UA_Server_browseNext_single(server, &adminSession, releaseContinuationPoint,
                                continuationPoint, &result);
    UA_RCU_UNLOCK();
    return result;
}

/***********************/
/* TranslateBrowsePath */
/***********************/

static void
walkBrowsePathElementNodeReference(UA_BrowsePathResult *result, size_t *targetsSize,
                                   UA_NodeId **next, size_t *nextSize, size_t *nextCount,
                                   UA_UInt32 elemDepth, UA_Boolean inverse, UA_Boolean all_refs,
                                   const UA_NodeId *reftypes, size_t reftypes_count,
                                   const UA_ReferenceNode *reference) {
    /* Does the direction of the reference match? */
    if(reference->isInverse != inverse)
        return;

    /* Is the node relevant? */
    if(!all_refs) {
        UA_Boolean match = false;
        for(size_t j = 0; j < reftypes_count; ++j) {
            if(UA_NodeId_equal(&reference->referenceTypeId, &reftypes[j])) {
                match = true;
                break;
            }
        }
        if(!match)
            return;
    }

    /* Does the reference point to an external server? Then add to the
     * targets with the right path "depth" */
    if(reference->targetId.serverIndex != 0) {
        UA_BrowsePathTarget *tempTargets =
            (UA_BrowsePathTarget *)UA_realloc(result->targets, sizeof(UA_BrowsePathTarget) * (*targetsSize) * 2);
        if(!tempTargets) {
            result->statusCode = UA_STATUSCODE_BADOUTOFMEMORY;
            return;
        }
        result->targets = tempTargets;
        (*targetsSize) *= 2;
        result->statusCode = UA_ExpandedNodeId_copy(&reference->targetId,
                                                    &result->targets[result->targetsSize].targetId);
        result->targets[result->targetsSize].remainingPathIndex = elemDepth;
        return;
    }

    /* Add the node to the next array for the following path element */
    if(*nextSize <= *nextCount) {
        UA_NodeId *tempNext = (UA_NodeId *)UA_realloc(*next, sizeof(UA_NodeId) * (*nextSize) * 2);
        if(!tempNext) {
            result->statusCode = UA_STATUSCODE_BADOUTOFMEMORY;
            return;
        }
        *next = tempNext;
        (*nextSize) *= 2;
    }
    result->statusCode = UA_NodeId_copy(&reference->targetId.nodeId,
                                        &(*next)[*nextCount]);
    ++(*nextCount);
}

static void
walkBrowsePathElement(UA_Server *server, UA_Session *session,
                      UA_BrowsePathResult *result, size_t *targetsSize,
                      const UA_RelativePathElement *elem, UA_UInt32 elemDepth,
                      const UA_QualifiedName *targetName,
                      const UA_NodeId *current, const size_t currentCount,
                      UA_NodeId **next, size_t *nextSize, size_t *nextCount) {
    /* Get the full list of relevant referencetypes for this path element */
    UA_NodeId *reftypes = NULL;
    size_t reftypes_count = 1; // all_refs or no subtypes => 1
    UA_Boolean all_refs = false;
    if(UA_NodeId_isNull(&elem->referenceTypeId)) {
        all_refs = true;
    } else if(!elem->includeSubtypes) {
        reftypes = (UA_NodeId*)(uintptr_t)&elem->referenceTypeId; // ptr magic due to const cast
    } else {
        const UA_Node *rootRef = UA_NodeStore_get(server->nodestore, &elem->referenceTypeId);
        if(!rootRef || rootRef->nodeClass != UA_NODECLASS_REFERENCETYPE)
            return;
        UA_StatusCode retval =
            getTypeHierarchy(server->nodestore, rootRef, false, &reftypes, &reftypes_count);
        if(retval != UA_STATUSCODE_GOOD)
            return;
    }

    /* Iterate over all nodes at the current depth-level */
    for(size_t i = 0; i < currentCount; ++i) {
        /* Get the node */
        const UA_Node *node = UA_NodeStore_get(server->nodestore, &current[i]);
        if(!node) {
            /* If we cannot find the node at depth 0, the starting node does not exist */
            if(elemDepth == 0)
                result->statusCode = UA_STATUSCODE_BADNODEIDUNKNOWN;
            continue;
        }

        /* Test whether the current node has the target name required in the
         * previous path element */
        if(targetName && (targetName->namespaceIndex != node->browseName.namespaceIndex ||
                          !UA_String_equal(&targetName->name, &node->browseName.name)))
            continue;

        /* Walk over the references in the node */
        /* Loop over the nodes references */
        for(size_t r = 0; r < node->referencesSize &&
                result->statusCode == UA_STATUSCODE_GOOD; ++r) {
            UA_ReferenceNode *reference = &node->references[r];
            walkBrowsePathElementNodeReference(result, targetsSize, next, nextSize, nextCount,
                                               elemDepth, elem->isInverse, all_refs,
                                               reftypes, reftypes_count, reference);
        }
    }

    if(!all_refs && elem->includeSubtypes)
        UA_Array_delete(reftypes, reftypes_count, &UA_TYPES[UA_TYPES_NODEID]);
}

/* This assumes that result->targets has enough room for all currentCount elements */
static void
addBrowsePathTargets(UA_Server *server, UA_Session *session, UA_BrowsePathResult *result,
                     const UA_QualifiedName *targetName, UA_NodeId *current, size_t currentCount) {
    for(size_t i = 0; i < currentCount; i++) {
        /* Get the node */
        const UA_Node *node = UA_NodeStore_get(server->nodestore, &current[i]);
        if(!node) {
            UA_NodeId_deleteMembers(&current[i]);
            continue;
        }

        /* Test whether the current node has the target name required in the
         * previous path element */
        if(targetName->namespaceIndex != node->browseName.namespaceIndex ||
           !UA_String_equal(&targetName->name, &node->browseName.name)) {
            UA_NodeId_deleteMembers(&current[i]);
            continue;
        }

        /* Move the nodeid to the target array */
        UA_BrowsePathTarget_init(&result->targets[result->targetsSize]);
        result->targets[result->targetsSize].targetId.nodeId = current[i];
        result->targets[result->targetsSize].remainingPathIndex = UA_UINT32_MAX;
        ++result->targetsSize;
    }
}

static void
walkBrowsePath(UA_Server *server, UA_Session *session, const UA_BrowsePath *path,
               UA_BrowsePathResult *result, size_t targetsSize,
               UA_NodeId **current, size_t *currentSize, size_t *currentCount,
               UA_NodeId **next, size_t *nextSize, size_t *nextCount) {
    UA_assert(*currentCount == 1);
    UA_assert(*nextCount == 0);

    /* Points to the targetName of the _previous_ path element */
    const UA_QualifiedName *targetName = NULL;

    /* Iterate over path elements */
    UA_assert(path->relativePath.elementsSize > 0);
    for(UA_UInt32 i = 0; i < path->relativePath.elementsSize; ++i) {
        walkBrowsePathElement(server, session, result, &targetsSize,
                              &path->relativePath.elements[i], i, targetName,
                              *current, *currentCount, next, nextSize, nextCount);

        /* Clean members of current */
        for(size_t j = 0; j < *currentCount; j++)
            UA_NodeId_deleteMembers(&(*current)[j]);
        *currentCount = 0;

        /* When no targets are left or an error occurred. None of next's
         * elements will be copied to result->targets */
        if(*nextCount == 0 || result->statusCode != UA_STATUSCODE_GOOD) {
            UA_assert(*currentCount == 0);
            UA_assert(*nextCount == 0);
            return;
        }

        /* Exchange current and next for the next depth */
        size_t tSize = *currentSize; size_t tCount = *currentCount; UA_NodeId *tT = *current;
        *currentSize = *nextSize; *currentCount = *nextCount; *current = *next;
        *nextSize = tSize; *nextCount = tCount; *next = tT;

        /* Store the target name of the previous path element */
        targetName = &path->relativePath.elements[i].targetName;
    }

    UA_assert(targetName != NULL);
    UA_assert(*nextCount == 0);

    /* After the last BrowsePathElement, move members from current to the
     * result targets */

    /* Realloc if more space is needed */
    if(targetsSize < result->targetsSize + (*currentCount)) {
        UA_BrowsePathTarget *newTargets =
            (UA_BrowsePathTarget*)UA_realloc(result->targets, sizeof(UA_BrowsePathTarget) *
                                             (result->targetsSize + (*currentCount)));
        if(!newTargets) {
            result->statusCode = UA_STATUSCODE_BADOUTOFMEMORY;
            for(size_t i = 0; i < *currentCount; ++i)
                UA_NodeId_deleteMembers(&(*current)[i]);
            *currentCount = 0;
            return;
        }
        result->targets = newTargets;
    }

    /* Move the elements of current to the targets */
    addBrowsePathTargets(server, session, result, targetName, *current, *currentCount);
    *currentCount = 0;
}

static void
translateBrowsePathToNodeIds(UA_Server *server, UA_Session *session,
                             const UA_BrowsePath *path, UA_BrowsePathResult *result) {
    if(path->relativePath.elementsSize <= 0) {
        result->statusCode = UA_STATUSCODE_BADNOTHINGTODO;
        return;
    }
        
    /* RelativePath elements must not have an empty targetName */
    for(size_t i = 0; i < path->relativePath.elementsSize; ++i) {
        if(UA_QualifiedName_isNull(&path->relativePath.elements[i].targetName)) {
            result->statusCode = UA_STATUSCODE_BADBROWSENAMEINVALID;
            return;
        }
    }

    /* Allocate memory for the targets */
    size_t targetsSize = 10; /* When to realloc; the member count is stored in
                              * result->targetsSize */
    result->targets = (UA_BrowsePathTarget*)UA_malloc(sizeof(UA_BrowsePathTarget) * targetsSize);
    if(!result->targets) {
        result->statusCode = UA_STATUSCODE_BADOUTOFMEMORY;
        return;
    }

    /* Allocate memory for two temporary arrays. One with the results for the
     * previous depth of the path. The other for the new results at the current
     * depth. The two arrays alternate as we descend down the tree. */
    size_t currentSize = 10; /* When to realloc */
    size_t currentCount = 0; /* Current elements */
    UA_NodeId *current = (UA_NodeId*)UA_malloc(sizeof(UA_NodeId) * currentSize);
    if(!current) {
        result->statusCode = UA_STATUSCODE_BADOUTOFMEMORY;
        UA_free(result->targets);
        return;
    }
    size_t nextSize = 10; /* When to realloc */
    size_t nextCount = 0; /* Current elements */
    UA_NodeId *next = (UA_NodeId*)UA_malloc(sizeof(UA_NodeId) * nextSize);
    if(!next) {
        result->statusCode = UA_STATUSCODE_BADOUTOFMEMORY;
        UA_free(result->targets);
        UA_free(current);
        return;
    }

    /* Copy the starting node into current */
    result->statusCode = UA_NodeId_copy(&path->startingNode, &current[0]);
    if(result->statusCode != UA_STATUSCODE_GOOD) {
        UA_free(result->targets);
        UA_free(current);
        UA_free(next);
        return;
    }
    currentCount = 1;

    /* Walk the path elements */
    walkBrowsePath(server, session, path, result, targetsSize,
                   &current, &currentSize, &currentCount,
                   &next, &nextSize, &nextCount);

    UA_assert(currentCount == 0);
    UA_assert(nextCount == 0);

    /* No results => BadNoMatch status code */
    if(result->targetsSize == 0 && result->statusCode == UA_STATUSCODE_GOOD)
        result->statusCode = UA_STATUSCODE_BADNOMATCH;

    /* Clean up the temporary arrays and the targets */
    UA_free(current);
    UA_free(next);
    if(result->statusCode != UA_STATUSCODE_GOOD) {
        for(size_t i = 0; i < result->targetsSize; ++i)
            UA_BrowsePathTarget_deleteMembers(&result->targets[i]);
        UA_free(result->targets);
        result->targets = NULL;
        result->targetsSize = 0;
    }
}

UA_BrowsePathResult
UA_Server_translateBrowsePathToNodeIds(UA_Server *server,
                                       const UA_BrowsePath *browsePath) {
    UA_BrowsePathResult result;
    UA_BrowsePathResult_init(&result);
    UA_RCU_LOCK();
    translateBrowsePathToNodeIds(server, &adminSession, browsePath, &result);
    UA_RCU_UNLOCK();
    return result;
}

void Service_TranslateBrowsePathsToNodeIds(UA_Server *server, UA_Session *session,
                                           const UA_TranslateBrowsePathsToNodeIdsRequest *request,
                                           UA_TranslateBrowsePathsToNodeIdsResponse *response) {
    UA_LOG_DEBUG_SESSION(server->config.logger, session, "Processing TranslateBrowsePathsToNodeIdsRequest");
    if(request->browsePathsSize <= 0) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADNOTHINGTODO;
        return;
    }

    size_t size = request->browsePathsSize;
    response->results = (UA_BrowsePathResult *)UA_Array_new(size, &UA_TYPES[UA_TYPES_BROWSEPATHRESULT]);
    if(!response->results) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;
        return;
    }

#ifdef UA_ENABLE_EXTERNAL_NAMESPACES
#ifdef NO_ALLOCA
    UA_Boolean isExternal[size];
    UA_UInt32 indices[size];
#else
    UA_Boolean *isExternal = UA_alloca(sizeof(UA_Boolean) * size);
    UA_UInt32 *indices = UA_alloca(sizeof(UA_UInt32) * size);
#endif /* NO_ALLOCA */
    memset(isExternal, false, sizeof(UA_Boolean) * size);
    for(size_t j = 0; j < server->externalNamespacesSize; ++j) {
        size_t indexSize = 0;
        for(size_t i = 0;i < size;++i) {
            if(request->browsePaths[i].startingNode.namespaceIndex != server->externalNamespaces[j].index)
                continue;
            isExternal[i] = true;
            indices[indexSize] = (UA_UInt32)i;
            ++indexSize;
        }
        if(indexSize == 0)
            continue;
        UA_ExternalNodeStore *ens = &server->externalNamespaces[j].externalNodeStore;
        ens->translateBrowsePathsToNodeIds(ens->ensHandle, &request->requestHeader, request->browsePaths,
                                           indices, (UA_UInt32)indexSize, response->results,
                                           response->diagnosticInfos);
    }
    response->resultsSize = size;
    for(size_t i = 0; i < size; ++i) {
        if(!isExternal[i])
            translateBrowsePathToNodeIds(server, session, &request->browsePaths[i],
                                         &response->results[i]);
    }
#else
    response->resultsSize = size;
    for(size_t i = 0; i < size; ++i)
        translateBrowsePathToNodeIds(server, session, &request->browsePaths[i],
                                     &response->results[i]);
#endif
}

void Service_RegisterNodes(UA_Server *server, UA_Session *session, const UA_RegisterNodesRequest *request,
                           UA_RegisterNodesResponse *response) {
    UA_LOG_DEBUG_SESSION(server->config.logger, session, "Processing RegisterNodesRequest");
    //TODO: hang the nodeids to the session if really needed
    response->responseHeader.timestamp = UA_DateTime_now();
    if(request->nodesToRegisterSize <= 0)
        response->responseHeader.serviceResult = UA_STATUSCODE_BADNOTHINGTODO;
    else {
        response->responseHeader.serviceResult =
            UA_Array_copy(request->nodesToRegister, request->nodesToRegisterSize,
                          (void**)&response->registeredNodeIds, &UA_TYPES[UA_TYPES_NODEID]);
        if(response->responseHeader.serviceResult == UA_STATUSCODE_GOOD)
            response->registeredNodeIdsSize = request->nodesToRegisterSize;
    }
}

void Service_UnregisterNodes(UA_Server *server, UA_Session *session, const UA_UnregisterNodesRequest *request,
                             UA_UnregisterNodesResponse *response) {
    UA_LOG_DEBUG_SESSION(server->config.logger, session, "Processing UnRegisterNodesRequest");
    //TODO: remove the nodeids from the session if really needed
    response->responseHeader.timestamp = UA_DateTime_now();
    if(request->nodesToUnregisterSize==0)
        response->responseHeader.serviceResult = UA_STATUSCODE_BADNOTHINGTODO;
}
