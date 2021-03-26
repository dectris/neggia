// SPDX-License-Identifier: MIT

#include "PathResolverV2.h"
#include <assert.h>
#include <memory>
#include "H5BTreeVersion2.h"
#include "H5FractalHeap.h"
#include "H5LocalHeap.h"
#include "constants.h"

#include <iostream>

PathResolverV2::PathResolverV2(const H5ObjectHeader& root) : _root(root) {}

ResolvedPath PathResolverV2::resolve(const H5Path& path) {
    return resolvePathInHeader(_root, path);
}

ResolvedPath PathResolverV2::resolvePathInHeader(const H5ObjectHeader& in,
                                                 const H5Path& path) {
    H5ObjectHeader parentEntry(path.isAbsolute() ? _root : in);
    for (auto itemIterator = path.begin(); itemIterator != path.end();
         ++itemIterator)
    {
        auto resolvedPath = findPathInObjectHeader(
                parentEntry, *itemIterator, H5Path(path, itemIterator + 1));
        if (resolvedPath.softLink) {
            return resolvePathInHeader(parentEntry, *resolvedPath.softLink);
        }
        if (resolvedPath.externalFile) {
            // the object is in a different file which must be opened by
            // upper layer
            return resolvedPath;
        }
        if ((itemIterator + 1) == path.end()) {
            return resolvedPath;
        }
        parentEntry = resolvedPath.objectHeader;
    }
    return ResolvedPath{parentEntry, {}, {}};
}

ResolvedPath PathResolverV2::findPathInLinkMsg(
        const H5ObjectHeader& parentEntry,
        const H5LinkMsg& linkMsg,
        const H5Path& remainingPath) {
    switch (linkMsg.linkType()) {
        case H5LinkMsg::SOFT: {
            H5Path targetPath(linkMsg.targetPath());
            return ResolvedPath{{},
                                {},
                                std::unique_ptr<H5Path>(new H5Path(
                                        targetPath + remainingPath))};
        }
        case H5LinkMsg::HARD: {
            return ResolvedPath{linkMsg.hardLinkObjectHeader(), {}, {}};
        }
        case H5LinkMsg::EXTERNAL: {
            std::string targetFile = linkMsg.targetFile();
            H5Path targetPath(linkMsg.targetPath());
            return ResolvedPath{
                    {},
                    std::unique_ptr<ResolvedPath::ExternalFile>{
                            new ResolvedPath::ExternalFile{
                                    targetFile, targetPath + remainingPath}},
                    {}};
        }
        default:
            throw std::runtime_error("unknown link type " +
                                     std::to_string(linkMsg.linkType()));
    }
}

ResolvedPath PathResolverV2::findPathInObjectHeader(
        const H5ObjectHeader& parentEntry,
        const std::string pathItem,
        const H5Path& remainingPath) {
    for (size_t i = 0; i < parentEntry.numberOfMessages(); ++i) {
        auto msg = parentEntry.headerMessage(i);
        switch (msg.type) {
            case H5LinkMsg::TYPE_ID: {
                H5LinkMsg linkMsg(msg.object);
                if (linkMsg.linkName() != pathItem)
                    continue;
                return findPathInLinkMsg(parentEntry, linkMsg, remainingPath);
            }
            case H5LinkInfoMsg::TYPE_ID: {
                H5LinkInfoMsg linkInfoMsg(msg.object);
                try {
                    auto linkMsg = linkInfoMsg.getLinkMessage(
                            _root.fileAddress(), pathItem);
                    return findPathInLinkMsg(parentEntry, linkMsg,
                                             remainingPath);
                } catch (const std::out_of_range&) {
                    continue;
                }
            }
        }
    }
    throw std::out_of_range("could not find " + pathItem);
}
