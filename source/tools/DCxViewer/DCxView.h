#pragma once

#include <MpqArchive.h>
#include <QObject>
#include <QScopedPointer>
#include <algorithm>
#include <dc6.h>

using WorldStone::DC6;

class QDC6Header : public QObject, public DC6::Header
{
    Q_OBJECT
public:
    Q_PROPERTY(int version MEMBER version NOTIFY updated);
    Q_PROPERTY(int sub_version MEMBER sub_version NOTIFY updated);
    Q_PROPERTY(int zeros MEMBER zeros NOTIFY updated);
    Q_PROPERTY(int directions MEMBER directions NOTIFY updated);
    Q_PROPERTY(int frames_per_dir MEMBER frames_per_dir NOTIFY updated);

    QDC6Header() : DC6::Header({}) {}
    QDC6Header(const DC6::Header& header) : DC6::Header(header) {}
    QDC6Header(const QDC6Header& header) : DC6::Header(header) {}
    QDC6Header& operator=(const QDC6Header& other)
    {
        DC6::Header::operator=(other);
        emit updated();
        return *this;
    }
signals:
    void updated();
};

Q_DECLARE_METATYPE(QDC6Header)

class QDC6FrameHeader : public QObject, public DC6::FrameHeader
{
    Q_OBJECT
public:
    Q_PROPERTY(int32_t flip MEMBER flip NOTIFY updated);
    Q_PROPERTY(int32_t width MEMBER width NOTIFY updated);
    Q_PROPERTY(int32_t height MEMBER height NOTIFY updated);
    Q_PROPERTY(int32_t offset_x MEMBER offset_x NOTIFY updated);
    Q_PROPERTY(int32_t offset_y MEMBER offset_y NOTIFY updated);
    Q_PROPERTY(int32_t zeros MEMBER zeros NOTIFY updated);
    Q_PROPERTY(int32_t next_block MEMBER next_block NOTIFY updated);
    Q_PROPERTY(int32_t length MEMBER length NOTIFY updated);
    QDC6FrameHeader() {}
    QDC6FrameHeader(const DC6::FrameHeader& frameHeader) : DC6::FrameHeader(frameHeader) {}
    QDC6FrameHeader(const QDC6FrameHeader& frameHeader) : DC6::FrameHeader(frameHeader) {}
    QDC6FrameHeader& operator=(const QDC6FrameHeader& other)
    {
        DC6::FrameHeader::operator=(other);
        return *this;
    }
signals:
    void updated();
};

Q_DECLARE_METATYPE(QDC6FrameHeader)

class QDC6 : public QObject, public DC6
{
    Q_OBJECT
public:
    QDC6() {}
    // This is dangerous, as it will not copy but whatever, it's fine for our usage
    QDC6(const QDC6& toCopy) {}
    QDC6& operator=(const QDC6& toCopy) {}
    ~QDC6() { Reset(); }

    Q_PROPERTY(QDC6Header* header READ getHeader NOTIFY updated);

    QDC6Header*      getHeader() { return &qHeader; }
    Q_INVOKABLE void Decode(QString filename, QString mpqFile);
    WorldStone::MpqArchive mpqArchive;
    QDC6Header             qHeader;
signals:
    void updated();
};
