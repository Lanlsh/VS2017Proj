/****************************************************************************
** Meta object code from reading C++ file 'CheckDose.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../CheckDose.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qplugin.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CheckDose.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CheckDose_t {
    QByteArrayData data[3];
    char stringdata0[31];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CheckDose_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CheckDose_t qt_meta_stringdata_CheckDose = {
    {
QT_MOC_LITERAL(0, 0, 9), // "CheckDose"
QT_MOC_LITERAL(1, 10, 19), // "slotContourProgress"
QT_MOC_LITERAL(2, 30, 0) // ""

    },
    "CheckDose\0slotContourProgress\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CheckDose[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   19,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void CheckDose::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CheckDose *_t = static_cast<CheckDose *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->slotContourProgress(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject CheckDose::staticMetaObject = {
    { &ACPluginBase::staticMetaObject, qt_meta_stringdata_CheckDose.data,
      qt_meta_data_CheckDose,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *CheckDose::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CheckDose::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CheckDose.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "Mantiea.AccuCheck.Plugins"))
        return static_cast< ACPluginInterface*>(this);
    return ACPluginBase::qt_metacast(_clname);
}

int CheckDose::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ACPluginBase::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}

QT_PLUGIN_METADATA_SECTION const uint qt_section_alignment_dummy = 42;

#ifdef QT_NO_DEBUG

QT_PLUGIN_METADATA_SECTION
static const unsigned char qt_pluginMetaData[] = {
    'Q', 'T', 'M', 'E', 'T', 'A', 'D', 'A', 'T', 'A', ' ', ' ',
    'q',  'b',  'j',  's',  0x01, 0x00, 0x00, 0x00,
    0xd4, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00,
    0xc0, 0x00, 0x00, 0x00, 0x1b, 0x03, 0x00, 0x00,
    0x03, 0x00, 'I',  'I',  'D',  0x00, 0x00, 0x00,
    0x19, 0x00, 'M',  'a',  'n',  't',  'i',  'e', 
    'a',  '.',  'A',  'c',  'c',  'u',  'C',  'h', 
    'e',  'c',  'k',  '.',  'P',  'l',  'u',  'g', 
    'i',  'n',  's',  0x00, 0x9b, 0x08, 0x00, 0x00,
    0x09, 0x00, 'c',  'l',  'a',  's',  's',  'N', 
    'a',  'm',  'e',  0x00, 0x09, 0x00, 'C',  'h', 
    'e',  'c',  'k',  'D',  'o',  's',  'e',  0x00,
    0x9a, ' ',  0xa1, 0x00, 0x07, 0x00, 'v',  'e', 
    'r',  's',  'i',  'o',  'n',  0x00, 0x00, 0x00,
    0x11, 0x00, 0x00, 0x00, 0x05, 0x00, 'd',  'e', 
    'b',  'u',  'g',  0x00, 0x95, 0x0f, 0x00, 0x00,
    0x08, 0x00, 'M',  'e',  't',  'a',  'D',  'a', 
    't',  'a',  0x00, 0x00, 'D',  0x00, 0x00, 0x00,
    0x05, 0x00, 0x00, 0x00, '<',  0x00, 0x00, 0x00,
    0x1b, 0x03, 0x00, 0x00, 0x04, 0x00, 'n',  'a', 
    'm',  'e',  0x00, 0x00, 0x09, 0x00, 'c',  'h', 
    'e',  'c',  'k',  'd',  'o',  's',  'e',  0x00,
    0x9b, 0x06, 0x00, 0x00, 0x07, 0x00, 'v',  'e', 
    'r',  's',  'i',  'o',  'n',  0x00, 0x00, 0x00,
    0x03, 0x00, '1',  '.',  '0',  0x00, 0x00, 0x00,
    0x0c, 0x00, 0x00, 0x00, '$',  0x00, 0x00, 0x00,
    0x0c, 0x00, 0x00, 0x00, 'l',  0x00, 0x00, 0x00,
    '4',  0x00, 0x00, 0x00, '`',  0x00, 0x00, 0x00,
    'P',  0x00, 0x00, 0x00
};

#else // QT_NO_DEBUG

QT_PLUGIN_METADATA_SECTION
static const unsigned char qt_pluginMetaData[] = {
    'Q', 'T', 'M', 'E', 'T', 'A', 'D', 'A', 'T', 'A', ' ', ' ',
    'q',  'b',  'j',  's',  0x01, 0x00, 0x00, 0x00,
    0xd4, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00,
    0xc0, 0x00, 0x00, 0x00, 0x1b, 0x03, 0x00, 0x00,
    0x03, 0x00, 'I',  'I',  'D',  0x00, 0x00, 0x00,
    0x19, 0x00, 'M',  'a',  'n',  't',  'i',  'e', 
    'a',  '.',  'A',  'c',  'c',  'u',  'C',  'h', 
    'e',  'c',  'k',  '.',  'P',  'l',  'u',  'g', 
    'i',  'n',  's',  0x00, 0x95, 0x08, 0x00, 0x00,
    0x08, 0x00, 'M',  'e',  't',  'a',  'D',  'a', 
    't',  'a',  0x00, 0x00, 'D',  0x00, 0x00, 0x00,
    0x05, 0x00, 0x00, 0x00, '<',  0x00, 0x00, 0x00,
    0x1b, 0x03, 0x00, 0x00, 0x04, 0x00, 'n',  'a', 
    'm',  'e',  0x00, 0x00, 0x09, 0x00, 'c',  'h', 
    'e',  'c',  'k',  'd',  'o',  's',  'e',  0x00,
    0x9b, 0x06, 0x00, 0x00, 0x07, 0x00, 'v',  'e', 
    'r',  's',  'i',  'o',  'n',  0x00, 0x00, 0x00,
    0x03, 0x00, '1',  '.',  '0',  0x00, 0x00, 0x00,
    0x0c, 0x00, 0x00, 0x00, '$',  0x00, 0x00, 0x00,
    0x1b, 0x13, 0x00, 0x00, 0x09, 0x00, 'c',  'l', 
    'a',  's',  's',  'N',  'a',  'm',  'e',  0x00,
    0x09, 0x00, 'C',  'h',  'e',  'c',  'k',  'D', 
    'o',  's',  'e',  0x00, '1',  0x00, 0x00, 0x00,
    0x05, 0x00, 'd',  'e',  'b',  'u',  'g',  0x00,
    0x9a, ' ',  0xa1, 0x00, 0x07, 0x00, 'v',  'e', 
    'r',  's',  'i',  'o',  'n',  0x00, 0x00, 0x00,
    0x0c, 0x00, 0x00, 0x00, '4',  0x00, 0x00, 0x00,
    0x88, 0x00, 0x00, 0x00, 0xa4, 0x00, 0x00, 0x00,
    0xb0, 0x00, 0x00, 0x00
};
#endif // QT_NO_DEBUG

QT_MOC_EXPORT_PLUGIN(CheckDose, CheckDose)

QT_WARNING_POP
QT_END_MOC_NAMESPACE
