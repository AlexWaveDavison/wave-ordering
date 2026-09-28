#pragma once

#include "database.h"

#include <QTemporaryDir>
#include <QtTest>

// Gives each test function its own fresh, seeded database file.
class TestDatabase
{
public:
    bool open()
    {
        close();
        m_dir = std::make_unique<QTemporaryDir>();
        return m_dir->isValid() && Database::initialize(path());
    }

    void close()
    {
        Database::close();
        m_dir.reset();
    }

    QString path() const { return m_dir ? m_dir->filePath(QStringLiteral("test.sqlite")) : QString(); }

private:
    std::unique_ptr<QTemporaryDir> m_dir;
};
