# Run Speech To Text Tool

This is a microservice to execute the Speech to Text Tool. Once this communication contract is published, it must not change so other services can rely on it.

This microservice uses the Multipart Form Data Parser written by Brian Khuu in C: 
https://github.com/mofosyne/minimal-multipart-form-data-parser-c

________________________________________________

## How to REQUEST Data

Since this program is not requesting data, rather, it is posting data using

HTTP Method: POST

URL: http://localhost:8001/api/upload

Required Header: Accept:application/json
Example Request (cURL):

curl -X POST -H 'Content-Type: video/mp4' -F "bob=@/home/ango/Downloads/Dream.mp4" http://localhost:1234/api/upload

___________________________________________________

## How to RECEIVE DATA

Expected Successful JSON Response (HTTP 200):

{"transcribe": "transcription summarized notes"}

____________________________________________________

## Required RESPONSE Format

|      Field     |   Type   |             Description               |
| -------------- | -------- | ------------------------------------- |
| transcription  |  String  | Transcribed data from the media file  |

_____________________________________________________

## UML Sequence Diagram

![alt text](UMLsequenceDiagram.png)

_____________________________________________________
## Limitations
- 100MB files (100,000,000 Bytes)
