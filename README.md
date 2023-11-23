# OpenLogReplicator

This project contains open source Oracle database CDC written purely in C++.
OpenLogReplicator reads transactions directly from database redo log files (parses binary files) and streams in JSON or Protobuf format to various targets.
The following targets are supported:
- Kafka
- flat file
- network stream (plain TCP/IP or ZeroMQ)

## Getting started

Refer to [OpenLogReplicator tutorials repository](https://github.com/bersler/OpenLogReplicator-tutorials) for a quick start with the project.

## Documentation

1. [Introduction to OpenLogReplicator](documentation/introduction/introduction.adoc)
2. [User Manual](documentation/user-manual/user-manual.adoc)
3. [Reference Manual](documentation/reference-manual/reference-manual.adoc)
4. [Installation Guide](documentation/installation/installation.adoc)
5. [Release Notes](documentation/release-notes/release-notes.adoc)
6. [Troubleshooting Guide](documentation/troubleshooting/troubleshooting.adoc)
7. [Log Message Guide](documentation/log-messages/log-messages.adoc)

Additionally:
1. [Tutorials](https://www.bersler.com/openlogreplicator/tutorials/)
2. [FAQ](https://www.bersler.com/openlogreplicator/faq/)
3. [Support](https://www.bersler.com/openlogreplicator/support/)
4. [Contribution](https://www.bersler.com/openlogreplicator/contribution/)

Use also [Gitter chat](https://gitter.im/bersler/OpenLogReplicator).

## Sponsoring the Project

If you (or your company) are benefiting from the project and would like to support the contributor, kindly support the project.

<a href="https://www.buymeacoffee.com/bersler" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/v2/default-blue.png" alt="Buy Me A Coffee" style="height: 40px !important;width: 160px !important;" ></a>
