/**
 * @author Vincent Wong (vincent.wong@jpl.nasa.gov)
 * @date   2016.10.13
 *
 * A wrapper around sqlite3 native's way of executing SQL queries. It functions
 * like a traditional SQL prepared statement.
 *
 * sqlite3's API is C-based, which means one must keep track of all the
 * allocated resources. This class wraps the most onerous parts and provides a
 * RAII-based method to initialize and execute statements and retrieve results.
 *
 * This code was original developed for the Self-Reliant Rovers TargetDB
 *
 */

#ifndef JPL_SYNOPSIS_Sqlite3Statement
#define JPL_SYNOPSIS_Sqlite3Statement

//std library
#include <string>

//sqlite3
#include <sqlite3.h>

namespace Synopsis {

  /**
   * Wrapper for sqlite3 statements
   *
   * @see Sqlite3Statement.h
   */
  class Sqlite3Statement
  {
    public: //'structors

    Sqlite3Statement(sqlite3* db, const std::string& sql);
    virtual ~Sqlite3Statement();

    public: //methods

    Sqlite3Statement& operator=(const Sqlite3Statement&) = delete;

    /**
     * Bind value to the prepared statement
     *
     * The statement must have bindable values (e.g. `?`) at the position
     * specified.
     *
     * @param[in] pos: the zero-based position of the bind variable
     * @param[in] val: the value to bind
     */
    void bind(int pos, int64_t val);

    /**
     * @see bind(int, int64_t)
     */
    void bind(int pos, int val);

    /**
     * @see bind(int, int64_t)
     */
    void bind(int pos, size_t val);

    /**
     * @see bind(int, int64_t)
     */
    void bind(int pos, double val);

    /**
     * @see bind(int, int64_t)
     */
    void bind(int pos, const std::string& val);

    /**
     * Fetch a column value from the executed query
     *
     * A previous call to step() MUST have returned SQLITE_ROW for fetch to
     * work. Otherwise, this method may throw an exception.
     *
     * @param[in] pos: the zero-based position of the result value
     * @return the result of the fetch. Its type depends on the
     * caller's template argument, although only a few types are defined and
     * can be called
     */
    template <typename T>
    T fetch(int pos);

    /**
     * Execute the statement by one step
     *
     * If the statement returns results, step() will return SQLITE_ROW to
     * indicate that there is a row to process. This continues until all rows
     * are consumed, in which case SQLITE_DONE will be returned. If the
     * statement has no results (e.g. insert), SQLITE_DONE will be immediately
     * returned.
     *
     * Any other sqlite3 conditions (unbound variable) will throw an exception.
     *
     * @return SQLITE_ROW or SQLITE_DONE
     *
     * @throw Sqlite3Exception
     */
    int step();

    /**
     * Resets the prepared statement for reuse
     *
     * Only usable if the statement has been returning SQLITE_ROW or
     * SQLITE_DONE. Otherwise, it will throw an exception.
     *
     * @throw Sqlite3Exception
     */
    void reset();

    /**
     * Throw exception if given sqlite3 error code
     *
     * Essentially a convenience wrapper for converting sqlite3 errors into
     * thrown exceptions, which descriptive error messages.
     *
     * @param[in] db: sqlite3 database connection
     * @param[in] rc: sqlite3 return code
     * @param[in] prefix: string to prefix to exception message
     *
     * @throw Sqlite3Exception
     */
    static void throwIfError(sqlite3* db, int rc, const std::string& prefix = "");

    protected: //methods

    /**
     * Throw exception if given sqlite3 error code
     *
     * @see throwIfError(sqlite*, rc, const string&)
     *
     * This variation uses the object's saved db connection
     *
     * @param[in] rc: sqlite3 return code
     * @param[in] prefix: string to prefix to exception message
     *
     * @throw Sqlite3Exception
     */
    void throwIfError(int rc, const std::string& prefix = "");

    protected: //members

    sqlite3* _db;
    sqlite3_stmt* _stmt;

  };
}

#endif
