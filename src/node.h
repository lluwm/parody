// Copyright (c) 2025 Lei Lu. All rights reserved.

#pragma once

#include <fstream>
#include <string>

typedef unsigned short int NodeNbr;

/**
 * A PARODY database consists of two files: the data file, which contains the
 * objects; and the index file, which contains the key indexes to the objects.
 * The Parody object opens or creates these two files when the program declares
 * the object. The data file has the file extension .dat, and the index file
 * has the file extension .idx. The file architecture supports:
 *
 * - Variable-length persistent objects.
 * - Indexes into and objects of multiple classes.
 * - Persistent objects that can grow or shrink in length or be deleted.
 * - Reuse of deleted file space.
 *
 * Both files in a PARODY database are managed by an input/output system of
 * fixed-length file nodes in a list. Nodes are logically addressed by node
 * numbers starting from 1.
 *
 * PARODY uses the node system in two ways. The index files are organized into
 * fixed-length nodes, which inherit the behavior of the Node class.
 * The objects are unformatted variable-length data streams that could extend
 * beyond the fixed-length node boundaries, so they declare Node objects and
 * copy their data into and out of the nodes.
 * 
 * Both files are represented by a FileHeader block at the beginning and a
 * a variable number of fixed-length data nodes.
 * 
 * FILE HEADER BLOCK
 * The header record contains two node numbers. The first node number points to
 * the first node in a chain of deleted nodes. The second node number points to
 * the highest node that has been allocated.
 * 
 * DATA NODES
 * Each file consists of a variable number of fixed-length nodes. The first node
 * is node number 1, and so on. Each node begins with a NodeNbr variable that
 * points to the next node in a logical list of nodes that form a set. The last
 * node in the set has a value of 0 in the NodeNbr variable. If the node has been
 * deleted by its user, the set is a list of nodes the first of which is pointed
 * to by the deleted node pointer in the FileHeader block. Nodes are implemented
 * as objects of the Node type.
 */


const short int sNODE_LENGTH = 128;
const short int sNODE_DATA_LENGTH = sNODE_LENGTH - sizeof(NodeNbr);


// Exceptions to be thrown.
class BadFileOpen {};
class FileReadError {};
class FileWriteError {};


/*************************
 * Node File Header Record
 *************************/
class FileHeader {
private:
    NodeNbr _deletedNode; // the first node in a chain of deleted nodes.
    NodeNbr _highestNode; // the highest node number that has been allocated.
public:
    FileHeader() {
        _deletedNode = _highestNode = 0;
    }

    void SetDeletedNode(NodeNbr num) {
        _deletedNode = num;
    }

    void SetHighestNode(NodeNbr num) {
        _highestNode = num;
    }

    NodeNbr GetDeletedNode() const {
        return _deletedNode;
    }

    NodeNbr GetHighestNode() const {
        return _highestNode;
    }
};


/*************************
 * Node File Header Class
 *************************/
class NodeFile {
private:
    FileHeader      _header;
    FileHeader      _origHeader;
    std::fstream    _nfile;
    bool            _newFile;   // true if building new node file.

public:
    NodeFile(const std::string& filename) throw (BadFileOpen);
    virtual ~NodeFile();

    void SetDeletedNode(NodeNbr num) {
        _header.SetDeletedNode(num);
    }

    NodeNbr DeletededNode() const {
        return _header.GetDeletedNode();
    }

    void setHightestNode(NodeNbr num) {
        _header.SetHighestNode(num);
    }

    NodeNbr HighestNode() const {
        return _header.GetHighestNode();
    }

    NodeNbr NewNode();

    void ReadData(void *buf, unsigned short size, long wh = -1) throw (FileReadError);
    void WriteData(const void *buf, unsigned short size, long wh = -1) throw (FileWriteError);
};


class Node {
private:
    NodeNbr _nextNode;
    NodeNbr _nodeNbr;       // current node number.
    bool    _nodeChanged;   // true, if the node changed.
    bool    _deleteNode;    // true, if the node is being deleted.
    NodeFile *_owner;

    void CloseNode();
public:
    Node(NodeFile *hd = nullptr, NodeNbr node = 0);
    virtual ~Node();

    NodeNbr GetNodeNbr() const {
        return _nodeNbr;
    }
    
    void MarkNodeChanged() {
        _nodeChanged = true;
    }

    void MarkNodeDeleted() {
        _deleteNode = true;
    }

    bool IsNodeChanged() const {
        return _nodeChanged;
    }

    NodeNbr NextNode() const {
        return _nextNode;
    }

    void SetNextNode(NodeNbr node) {
        _nextNode = node;
        MarkNodeChanged();
    }

    Node& operator=(Node& node);
    long NodeAddress();
};