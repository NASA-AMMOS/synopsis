/**
* @author Vincent Wong (vincent.wong@jpl.nasa.gov)
* @date   2016.10.11
*
* @see Sqlite3Statement.hpp
**/

#include "Sqlite3Statement.hpp"
#include<iostream>

namespace Synopsis {

  Sqlite3Statement::Sqlite3Statement(
      sqlite3* db,
      const std::string& sql
    )
    : _db(db)
  {
    int rc;
    rc = sqlite3_prepare_v2(_db, sql.c_str(), sql.length()+1, &_stmt, NULL);
    throwIfError(rc);
  }

  Sqlite3Statement::~Sqlite3Statement()
  {
    sqlite3_finalize(_stmt);
  }

  void Sqlite3Statement::bind(int pos, int64_t val)
  {
    int rc;
    rc = sqlite3_bind_int64(_stmt, pos+1, static_cast<sqlite3_int64>(val));
    throwIfError(rc);
  }

  void Sqlite3Statement::bind(int pos, int val)
  {
    bind(pos, (int64_t) val);
  }

  void Sqlite3Statement::bind(int pos, size_t val)
  {
    bind(pos, (int64_t) val);
  }

  void Sqlite3Statement::bind(int pos, double val)
  {
    int rc;
    rc = sqlite3_bind_double(_stmt, pos+1, val);
    throwIfError(rc);
  }

  void Sqlite3Statement::bind(int pos, const std::string& val)
  {
    int rc;
    rc = sqlite3_bind_text(_stmt, pos+1, val.c_str(), val.length()+1, SQLITE_TRANSIENT);
    throwIfError(rc);
  }

  int Sqlite3Statement::step()
  {
    int rc;
    rc = sqlite3_step(_stmt);
    throwIfError(rc);
    return rc;
  }

  void Sqlite3Statement::reset()
  {
    int rc;
    rc = sqlite3_reset(_stmt);
    throwIfError(rc);
    rc = sqlite3_clear_bindings(_stmt);
    throwIfError(rc);
  }

  template <>
  std::string Sqlite3Statement::fetch<std::string>(int pos)
  {
    return std::string(
      reinterpret_cast<const char*>(
        sqlite3_column_text(_stmt, pos)));
  }

  template <>
  int64_t Sqlite3Statement::fetch<int64_t>(int pos)
  {
    return sqlite3_column_int64(_stmt, pos);
  }

  template <>
  int Sqlite3Statement::fetch<int>(int pos)
  {
    return sqlite3_column_int(_stmt, pos);
  }

  template <>
  double Sqlite3Statement::fetch<double>(int pos)
  {
    return sqlite3_column_double(_stmt, pos);
  }

  void Sqlite3Statement::throwIfError(sqlite3* db, int rc, const std::string& prefix)
  {
    if (rc != SQLITE_ROW && rc != SQLITE_OK && rc != SQLITE_DONE) {
      // std::cout <<  "SQL error: " << std::string(sqlite3_errmsg(db)) << std::endl;
      throw (prefix + std::string(sqlite3_errmsg(db)));
    }
  }

  void Sqlite3Statement::throwIfError(int rc, const std::string& prefix)
  {
    throwIfError(_db, rc, prefix);
  }
}
