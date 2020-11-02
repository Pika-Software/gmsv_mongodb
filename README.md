# MongoDB Wrapper for Garry's mod
Simple MongoDB Wrapper for Garry's mod

**DOCUMENTATION IS OUTDATED!**
## Table of contents
* [Features](#features)
* [Requirements](#requirements)
* [How to install](#how-to-install)
* [How to use](#how-to-use)
## Features
* Basic functionality for working with MongoDB
* Converting BSON to table and back
* Support for Windows and Linux (Tested only on Windows 10 and Ubuntu 20.04)
#### To Do:
* Make asynchronous version
* Add more functionality
* Support for x64 version
* Compatible with OSX
* Better Linux compatibility (i'm compiling 32-bit library for linux using ubuntu 16.04 i386...)
* Better memory management
* Make wiki
## Requirements
#### For building source:
 * [MongoDB C Driver](https://github.com/mongodb/mongo-c-driver)
 * [MongoDB C++ Driver](https://github.com/mongodb/mongo-cxx-driver) with static libs
#### For Linux:
 * If you are using a release, you need to download `libssl1.0.0`
 * `libssl-dev` for Debian/Ubuntu, `openssl-devel` for RedHat/Fedora. 
 * `libsasl2-dev`for Debian/Ubuntu, `cyrus-sasl-devel` for RedHat/Fedora.
## How to install
1. Download lastest release from [releases](https://github.com/Pika-Software/gmsv_mongodb/releases).
If your server is on Linux download `gmsv_mongodb_linux.dll`. If on Windows download `gmsv_mongodb_win32.dll`.

2. Downloaded DLL must be placed in the directory `<Garrysmod path>/garrysmod/lua/bin/`. If there is no `bin` folder, then just create it.

3. If your server is on 64-bit ubuntu, you must enter the following commands:
```sh
# Enable the i386 architecture
$ sudo dpkg --add-architecture i386

# If you are using Ubuntu above version 16.04 you should follow these steps:
$ sudo nano /etc/apt/sources.list

# In the file add the following line:
deb http://security.ubuntu.com/ubuntu xenial-security main
# And close the file with the keyboard shortcut: ctrl+s and ctrl+x

# Update repositories and install libssl-dev, libssl1.0.0 and libsasl2-dev
$ sudo apt update
$ sudo apt install libssl-dev:i386 libssl1.0.0:i386 libsasl2-dev:i386
```
## How to use
#### Load MongoDB wrapper and create a client:
```lua
require("mongodb") -- Loading wrapper.

-- Creating MongoDB Client
-- Args:
--      connection_string (string): MongoDB connection string. See https://docs.mongodb.com/manual/reference/connection-string/
-- Returns:
--         MongoDB Client: Returns MongoDB client
local client = mongodb.CreateClient("mongodb://localhost") -- Example connecting to mongodb server at localhost with standard port (27017).
```
#### Working with MongoDB client
```lua
-- Returns client status
-- Returns:
--         number: Current client status. Could be one of these statuses:
--          client.STATUS_DISCONNECTED = 1: Client not connected to server
--          client.STATUS_CONNECTING = 2: Client connects to server
--          client.STATUS_CONNECTED = 3: Client successfully connected to server
--          client.STATUS_FAILED = 4: Сlient was unable to connect to the server
--          client.STATUS_DESTROYED = 5: The client is invalid, using any functions will throw an error, except the status function.
local status = client:Status(Status)

-- Connect to MongoDB Server
-- Exceptions:
--            1. When failed to connect to the server.
client:Connect()

-- Disconnect from the server
-- Warning! Calling this function will destroy client. if you need to reconnect, you need to create a new client.
client:Disconnect()

-- Returns list of databases
-- Returns:
--         table: List of databases
-- Exceptions:
--            1. Failed to get the list of databases.
--            2. Failed to convert BSON to table
local databases = client:ListDatabases()

-- Returns MongoDB database
-- Args:
--      db_name (string): Database name
-- Returns:
--         MongoDB Database: The MongoDB Database
local db = client:Database("test") -- Returns database with name "test". If database not exists, creates it.
```
#### Working with database
```lua
-- Returns database name
-- Returns:
--         name (string): The name of database.
local name = db:Name()

-- Drops database
-- Exceptions:
--            1. Failed to drop database
db:Drop()

-- Checks if there is a collection in the database
-- Args:
--      collection_name (string): Name of collection
-- Returns:
--         bool: True if collection exists, otherwise false
-- Exceptions:
--            1. If function "ListFunction" fails
local isExists = db:HasCollection("my_collection")

-- Returns list of collections
-- Returns:
--         table: List of collections
-- Exceptions:
--            1. Failed to get the list of collections.
--            2. Failed to convert BSON to table
local collections = db:ListCollections()

-- Returns MongoDB Collection
-- Args:
--      collection_name (string): Name of collection
-- Returns:
--         MongoDB Collection: The MongoDB Collection
local coll = db:Collection("my_collection") -- Returns collection from database with name "my_collection"
```
#### Working with MongoDB collections
```lua
-- Inserting one document into collection
-- Result can be nil, if no result
-- Args:
--      document (table)
-- Returns:
--         table | nil: Result of inserting document (Tommorow, i will write wiki, where will be example of result structure)
-- Exceptions:
--            1. Failed to insert document
local result = coll:InsertOne({
    name = "Garry",
    id = 102,
    money = 1000000000,
})

-- Inserting many documents into collection
-- Result can be nil, if no result
-- Args:
--      document (table)
--      ...[documents] (table)
-- Returns:
--         table | nil: Result of inserting document
-- Exceptions:
--            1. Failed to insert documents
local result = coll:InsertMany({
    name = "Garry",
    id = 102,
    level = 999,
}, {
    name = "code_gs",
    id = 103,
    level = 999,
})

-- Returns one document from collection
-- Result can be nil, if nothing found
-- Args:
--      filter (table)
-- Returns:
--         table | nil: Founded document
-- Exceptions:
--            1. Failed to find document
--            2. Failed to parse document
local result = coll:FindOne({id = 103}) -- Will return document of user "code_gs".

-- Returns documents from collection
-- Args:
--      filter (table)
-- Returns:
--         table: List of founded documents. Can be empty table
-- Exceptions:
--            1. Failed to find documents
--            2. Failed to parse documents
local result = coll:Find({id = 103}) -- Will return document of user "code_gs".

-- Updates one document, which matched the filter.
-- Result can be nil, if no result
-- Args:
--      filter (table)
--      params (table)
-- Returns:
--         table | nil: Result of updating document
-- Exceptions:
--            1. Failed to update document
--            2. Filter or params is invalid.
local result = coll:UpdateOne({id = 103}, { ["$set"] = {level = 1001} })

-- Updates many documents, which matched the filter.
-- Result can be nil, if no result
-- Args:
--      filter (table)
--      params (table)
-- Returns:
--         table | nil: Result of updating documents
-- Exceptions:
--            1. Failed to update document
--            2. Filter or params is invalid.
local result = coll:UpdateMany({id = 102}, { ["$set"] = {level = 1002} })

-- Deletes one document, which matched the filter.
-- Result can be nil, if no result
-- Args:
--      filter (table)
-- Returns:
--         table | nil: Result of deleting document
-- Exceptions:
--            1. Failed to delete document
local result = coll:DeleteOne({id = 103})

-- Deletes many documents, which matched the filter.
-- Result can be nil, if no result
-- Args:
--      filter (table)
-- Returns:
--         table | nil: Result of deleting documents
-- Exceptions:
--            1. Failed to delete documents
local result = coll:DeleteMany({id = 103})

-- Creates index.
-- Args:
--      keys (table)
--      [options] (table)
-- Returns:
--         table: Returns index?
-- Exceptions:
--            1. Failed to create index.
--            2. Failed to parse document.
local result = coll:CreateIndex({id = 103})
```
