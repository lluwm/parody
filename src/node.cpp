#include "node.h"
#include <string>
#include <unistd.h>

using namespace std;

NodeFile::NodeFile(const string& filename) throw (BadFileOpen)
{
    // Check if file already exists.
    _newFile = access(filename.c_str(), F_OK) != 0;

    // File open for reading, writing and in binary mode.
    _nfile.open(filename.c_str(), ios::in | ios::out | ios::binary);
    if (_nfile.fail()) {
        throw BadFileOpen();
    }

    if (!_newFile) {
        // an eixsting file, read the header.
        ReadData(&_header, sizeof(_header));
    } else {
        // creating the file, write the empty header.
        WriteData(&_header, sizeof(_header));
    }
    _origHeader = _header;
}

NodeFile::~NodeFile()
{

}

void
NodeFile::ReadData(void *buf, unsigned short size, long wh) throw (FileReadError)
{
    if (wh != -1) {
        // Set input position.
        _nfile.seekg(wh);
    }

    _nfile.read(reinterpret_cast<char *>(buf), size);
    if (_nfile.fail() || _nfile.eof()) {
        // Clear error flags.
        _nfile.clear();
        throw FileReadError();
    }
    // Set output position.
    _nfile.seekp(_nfile.tellg());
}

void
NodeFile::WriteData(const void *buf, unsigned short size, long wh) throw (FileWriteError)
{
    if (wh != -1) {
        // Set output position.
        _nfile.seekp(wh);
    }

    _nfile.write(reinterpret_cast<const char *>(buf), size);
    if (_nfile.fail()) {
        _nfile.clear();
        throw FileWriteError();
    }

    // Set input position.
    _nfile.seekg(_nfile.tellp());
}

NodeNbr
NodeFile::NewNode()
{
    NodeNbr newNode;
    if (_header.GetDeletedNode()) {
        // Reuse deleted node number.
        newNode = _header.GetDeletedNode();
        Node node(this, newNode);
        _header.SetDeletedNode(node.NextNode());
        node.SetNextNode(0);
    } else {
        // highest node number + 1.
        newNode = _header.GetHighestNode() + 1;
    }
    return newNode;
}

Node::Node(NodeFile *hd, NodeNbr node)
{
    _nextNode = 0;
    _nodeChanged = _deleteNode = false;
    _nodeNbr = node;
    _owner = hd;
    if (_nodeNbr) {
        long addr = NodeAddress();
        // Read the header.
        
        try {
            _owner->ReadData(&_nextNode, sizeof(_nextNode), addr);
        } catch (FileReadError&) {
            // Handle read error.
            _owner->WriteData(&_nextNode, sizeof(_nextNode), addr);
        }
    }
}

long
Node::NodeAddress()
{
    // Compute the node's address.
    long addr = _nodeNbr - 1;
    addr *= sNODE_LENGTH;
    addr += sizeof(FileHeader);
    return addr;
}

Node::~Node()
{
    CloseNode();
}

void
Node::CloseNode()
{
    if (_owner && _nodeNbr && (_nodeChanged || _deleteNode)) {
        if (_deleteNode) {
            // If the node is being deleted, add it to the deleted node list.
            _nextNode = _owner->DeletededNode();
            _owner->SetDeletedNode(_nodeNbr);
        }
        long addr = NodeAddress();

        // Write the header back to the file.
        _owner->WriteData(&_nextNode, sizeof(_nextNode), addr);

        if (_deleteNode) {
            // Zero out the deleted node's data.
            char zeroBuffer[sNODE_DATA_LENGTH] = { 0 };
            memset(zeroBuffer, 0, sNODE_DATA_LENGTH);
            // Mark as deleted.
            zeroBuffer[0] = -1;
            _owner->WriteData(zeroBuffer, sNODE_DATA_LENGTH);
        }
    }
}
