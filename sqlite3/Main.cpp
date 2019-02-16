#include <stdio.h>
#include <process.h>
#include <string>
#include "ncbind/ncbind.hpp"
#include "sqlite3.h"
#include "sqlite3_xp3_vfs/xp3_vfs.h"


// ���b�Z�[�W�R�[�h
#define	WM_SQLITE_STATECHANGE (WM_APP+8)
#define	WM_SQLITE_PROGRESS    (WM_APP+9)
#define PROGRESS_COUNT 100

/**
 * �X�e�[�g�����g tTJSVariant ���o�C���h
 * @param stmt �X�e�[�g�����g
 * @param param �o�C���h����f�[�^
 * @param pos �o�C���h�ʒu
 * @return �G���[�R�[�h
 */
static int
bindParam(sqlite3_stmt *stmt, const tTJSVariant &param, int pos)
{
	switch (param.Type()) {
	case tvtInteger:
		return sqlite3_bind_int64(stmt, pos, param);
	case tvtReal:
		return sqlite3_bind_double(stmt, pos, param);
	case tvtString:
		{
			tTJSVariantString *str = param.AsStringNoAddRef();
			return sqlite3_bind_text16(stmt, pos, *str, str->GetLength() * sizeof tjs_char, SQLITE_TRANSIENT);
		}
	case tvtOctet:
		{
			tTJSVariantOctet *octet = param.AsOctetNoAddRef();
			return sqlite3_bind_blob(stmt, pos, octet->GetData(), octet->GetLength(), SQLITE_STATIC);
		}
	default:
		return sqlite3_bind_null(stmt, pos);
	}
}

/**
 * �o�C���h�ʒu���擾
 * @param name �p�����[�^��
 * @return �o�C���h�ʒu
 */
static int
getBindPos(sqlite3_stmt *stmt, const tTJSVariant &name)
{
	switch (name.Type()) {
	case tvtInteger:
	case tvtReal:
		return (int)name + 1;
	case tvtString:
		{
			int ret = 0;
			const tjs_char *n = name.GetString();
			int	len = ::WideCharToMultiByte(CP_UTF8, 0, n, -1, NULL, 0, NULL, NULL);
			if (len > 0) {
				char *buf = new char[len + 1];
				::WideCharToMultiByte(CP_UTF8, 0, n, -1, buf, len, NULL, NULL);
				buf[len] = '\0';
				ret =sqlite3_bind_parameter_index(stmt, buf);
				delete[] buf;
			}
			return ret;
		}
	default:
		return 0;
	}
}

/**
 * �o�C���h�����̌Ăяo���p
 */
class BindCaller : public tTJSDispatch /** EnumMembers �p */
{
protected:
	sqlite3_stmt *stmt;
	int errorCode;
public:
	BindCaller(sqlite3_stmt *stmt) : stmt(stmt), errorCode(SQLITE_OK) {};
	int getErrorCode() {
		return errorCode;
	}
	virtual tjs_error TJS_INTF_METHOD FuncCall( // function invocation
												tjs_uint32 flag,			// calling flag
												const tjs_char * membername,// member name ( NULL for a default member )
												tjs_uint32 *hint,			// hint for the member name (in/out)
												tTJSVariant *result,		// result
												tjs_int numparams,			// number of parameters
												tTJSVariant **param,		// parameters
												iTJSDispatch2 *objthis		// object as "this"
												) {
		if (numparams > 1) {
			tTVInteger flag = param[1]->AsInteger();
			if (!(flag & TJS_HIDDENMEMBER)) {
				errorCode = bindParam(stmt, *param[1], getBindPos(stmt, *param[0]));
			}
		}
		if (result) {
			*result = errorCode == SQLITE_OK;
		}
		return TJS_S_OK;
	}
};

/**
 * �����p�����[�^���o�C���h
 * @param stmt �X�e�[�g�����g
 * @param params �����p�����[�^�̔z��܂��͎���
 * @return �G���[�R�[�h
 */
static int
bindParams(sqlite3_stmt *stmt, const tTJSVariant &params)
{
	tTJSVariantClosure &vc = params.AsObjectClosureNoAddRef();
	if (vc.IsInstanceOf(TJS_IGNOREPROP,NULL,NULL,L"Array",NULL) == TJS_S_TRUE) {
		int ret = SQLITE_OK;
		params.AsObjectClosureNoAddRef();
		tTJSVariant count;
		vc.PropGet(0, L"count", NULL, &count, NULL);
		int cnt = count;
		for (int i=0;i<cnt;i++) {
			tTJSVariant value;
			vc.PropGetByNum(0, i, &value, NULL);
			if ((ret = bindParam(stmt, value, i+1)) != SQLITE_OK) {
				break;
			}
		}
		return ret;
	} else {
		BindCaller *caller = new BindCaller(stmt);
		tTJSVariantClosure closure(caller);
		vc.EnumMembers(TJS_IGNOREPROP, &closure, NULL);
		int errorCode = caller->getErrorCode();
		caller->Release();
		return errorCode;
	}
}

/**
 * �X�e�[�g�����g���� tTJSVariant ���擾
 * @param stmt �X�e�[�g�����g
 * @param variant �i�[��
 * @param num �J�����ԍ�
 */
static void
getColumnData(sqlite3_stmt *stmt, tTJSVariant &variant, int num)
{
	switch (sqlite3_column_type(stmt, num)) {
	case SQLITE_INTEGER:
		variant = sqlite3_column_int64(stmt, num);
		break;
	case SQLITE_FLOAT:
		variant = sqlite3_column_double(stmt, num);
		break;
	case SQLITE_TEXT:
		variant = (const tjs_char *)sqlite3_column_text16(stmt, num);
		break;
	case SQLITE_BLOB:
		variant = tTJSVariant((const tjs_uint8 *) sqlite3_column_blob(stmt, num), sqlite3_column_bytes(stmt, num));
		break;
	default:
		variant.Clear();
		break;
	}
}

extern void initContainFunc(sqlite3 *db);

/**
 * Sqlite�N���X
 */
class Sqlite {

public:
	/**
	 * �R���X�g���N�^
	 * @param database �f�[�^�x�[�X�t�@�C����
	 * @param readonly �ǂݍ��ݐ�p
	 */
	Sqlite(const tjs_char *database, bool readonly=false) : db(NULL) {
		if (readonly) {
			// �ǂݍ��ݐ�p���͋g���g���̏����n���g��
			int	len = ::WideCharToMultiByte(CP_UTF8, 0, database, -1, NULL, 0, NULL, NULL);
			if (len > 0) {
				char *buf = new char[len + 1];
				::WideCharToMultiByte(CP_UTF8, 0, database, -1, buf, len, NULL, NULL);
				buf[len] = '\0';
				sqlite3_open_v2(buf, &db, SQLITE_OPEN_READONLY, "xp3");
				delete[] buf;
			}
		} else {
			// �����łȂ��ꍇ��OS�̏����n���g��
			ttstr filename = database;
			if (*database == '\0' || *database == ':') {
				// �󕶎��܂��� ':' �Ŏn�܂�ꍇ�� sqlite �̓���Ȉ���
				sqlite3_open16(database, &db);
			} else {
				filename = TVPNormalizeStorageName(filename);
				ttstr localname(TVPGetLocallyAccessibleName(filename));
				if (filename.length() && localname.length()) {
					sqlite3_open16(localname.c_str(), &db);
				} else {
					TVPThrowExceptionMessage(L"Unable to open the database file, try readonly if exists: %1", filename);;
				}
			}
		}
		if (db) {
			initContainFunc(db);
		}
	}

	/**
	 * �f�X�g���N�^
	 */
	~Sqlite() {
		if (db) {
			sqlite3_close(db);
		}
	}

	/**
	 * SQL�����s����
	 */
	static tjs_error exec(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, Sqlite *self) {
		if (numparams < 1) {
			return TJS_E_BADPARAMCOUNT;
		}
		sqlite3_stmt *stmt = NULL;
		int ret;
		if ((ret = sqlite3_prepare16_v2(self->db, params[0]->GetString(), -1, &stmt, NULL)) == SQLITE_OK) {
			sqlite3_reset(stmt);
			if (numparams <= 1 || (ret = ::bindParams(stmt, *params[1])) == SQLITE_OK) {
				if (numparams > 2 && params[2]->Type() == tvtObject) {
					tTJSVariantClosure &callback = params[2]->AsObjectClosureNoAddRef();
					// �J������
					int argc = sqlite3_column_count(stmt);
					// ����������
					tTJSVariant **args = new tTJSVariant*[argc];
					for (int i=0;i<argc;i++) {
						args[i] = new tTJSVariant();
					}
					while ((ret = sqlite3_step(stmt)) == SQLITE_ROW) {
						// �����ɓW�J
						for (int i=0;i<argc;i++) {
							::getColumnData(stmt, *args[i], i);
						}
						callback.FuncCall(0, NULL, NULL, NULL, argc, args, NULL);
					}
					// �����j��
					for (int i=0;i<argc;i++) {
						delete args[i];
					}
					delete[] args;
				} else {
					while ((ret = sqlite3_step(stmt)) == SQLITE_ROW);
				}
			}
			sqlite3_finalize(stmt);
		}
		if (result) {
			*result = ret == SQLITE_OK || ret == SQLITE_DONE;
		}
		return TJS_S_OK;
	}

	/**
	 * �l�擾�p��SQL�����s����B
	 */
	static tjs_error execValue(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, Sqlite *self) {
		if (numparams < 1) {
			return TJS_E_BADPARAMCOUNT;
		}
		sqlite3_stmt *stmt = NULL;
		if (sqlite3_prepare16_v2(self->db, params[0]->GetString(), -1, &stmt, NULL) == SQLITE_OK) {
			sqlite3_reset(stmt);
			if (numparams <= 1 || ::bindParams(stmt, *params[1]) == SQLITE_OK) {
				int argc = sqlite3_column_count(stmt);
				if (sqlite3_step(stmt) == SQLITE_ROW && argc > 0) {
					if (result) {
						::getColumnData(stmt, *result, 0);
					}
				}
				sqlite3_finalize(stmt);
			}
		}
		return TJS_S_OK;
	}
	
	bool begin() {
		return sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL) == SQLITE_OK;
	}
	bool commit() {
		return sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL) == SQLITE_OK;
	}
	bool rollback() {
		return sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL) == SQLITE_OK;
	}

	tjs_int64 getLastInsertRowId() const {
		return db ? sqlite3_last_insert_rowid(db) : 0;
	}

	int getErrorCode() const {
		return db ? sqlite3_errcode(db) : -1;
	}
	
	ttstr getErrorMessage() const {
		return db ? ttstr((const tjs_char*)sqlite3_errmsg16(db)) : ttstr("database open failed");
	}
	
	/**
	 * �C���X�^���X�����t�@�N�g��
	 */
	static tjs_error factory(Sqlite **result, tjs_int numparams, tTJSVariant **params, iTJSDispatch2 *objthis) {
		if (numparams < 1) {
			return TJS_E_BADPARAMCOUNT;
		}
		*result = new Sqlite(params[0]->GetString(), numparams > 1 ? (int)*params[1] != 0 : false);
		return TJS_S_OK;
	}
	
	sqlite3 *getDatabase() const {
		return db;
	}
	
private:
	sqlite3 *db;
};

#define ENUM(n) Variant(#n, (int)n)


NCB_REGISTER_CLASS(Sqlite) {
	ENUM(SQLITE_OK); /* Successful result */
	ENUM(SQLITE_ERROR); /* SQL error or missing database */
	ENUM(SQLITE_INTERNAL); /* Internal logic error in SQLite */
	ENUM(SQLITE_PERM); /* Access permission denied */
	ENUM(SQLITE_ABORT); /* Callback routine requested an abort */
	ENUM(SQLITE_BUSY); /* The database file is locked */
	ENUM(SQLITE_LOCKED); /* A table in the database is locked */
	ENUM(SQLITE_NOMEM); /* A malloc() failed */
	ENUM(SQLITE_READONLY); /* Attempt to write a readonly database */
	ENUM(SQLITE_INTERRUPT); /* Operation terminated by sqlite3_interrupt()*/
	ENUM(SQLITE_IOERR); /* Some kind of disk I/O error occurred */
	ENUM(SQLITE_CORRUPT); /* The database disk image is malformed */
	ENUM(SQLITE_NOTFOUND); /* NOT USED. Table or record not found */
	ENUM(SQLITE_FULL); /* Insertion failed because database is full */
	ENUM(SQLITE_CANTOPEN); /* Unable to open the database file */
	ENUM(SQLITE_PROTOCOL); /* NOT USED. Database lock protocol error */
	ENUM(SQLITE_EMPTY); /* Database is empty */
	ENUM(SQLITE_SCHEMA); /* The database schema changed */
	ENUM(SQLITE_TOOBIG); /* String or BLOB exceeds size limit */
	ENUM(SQLITE_CONSTRAINT); /* Abort due to constraint violation */
	ENUM(SQLITE_MISMATCH); /* Data type mismatch */
	ENUM(SQLITE_MISUSE); /* Library used incorrectly */
	ENUM(SQLITE_NOLFS); /* Uses OS features not supported on host */
	ENUM(SQLITE_AUTH); /* Authorization denied */
	ENUM(SQLITE_FORMAT); /* Auxiliary database format error */
	ENUM(SQLITE_RANGE); /* 2nd parameter to sqlite3_bind out of range */
	ENUM(SQLITE_NOTADB); /* File opened that is not a database file */
	ENUM(SQLITE_ROW); /* sqlite3_step() has another row ready */
	ENUM(SQLITE_DONE); /* sqlite3_step() has finished executing */
	Factory(&ClassT::factory);
	RawCallback(TJS_W("exec"), &Class::exec, 0);
	RawCallback(TJS_W("execValue"), &Class::execValue, 0);
	NCB_METHOD(begin);
	NCB_METHOD(commit);
	NCB_METHOD(rollback);
	NCB_PROPERTY_RO(lastInsertRowId, getLastInsertRowId);
	NCB_PROPERTY_RO(errorCode, getErrorCode);
	NCB_PROPERTY_RO(errorMessage, getErrorMessage);
};

/**
 * Sqlite�̃X�e�[�g�����g�������N���X
 */
class SqliteStatement {

public:
	/**
	 * �R���X�g���N�^
	 */
	SqliteStatement(tTJSVariant &sqlite) : sqlite(sqlite), db(NULL), stmt(NULL) {
		Sqlite *sq = ncbInstanceAdaptor<Sqlite>::GetNativeInstance(sqlite.AsObjectNoAddRef());
		if (sq) {
			db = sq->getDatabase();
		}
	}

	/**
	 * �f�X�g���N�^
	 */
	~SqliteStatement() {
		close();
	}

	// �X�e�[�g���J��
	static tjs_error open(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, SqliteStatement *self) {
		if (numparams < 1) {
			return TJS_E_BADPARAMCOUNT;
		}
		int ret = self->_open(params[0]->GetString(), numparams > 0 ? params[1]: NULL);
		if (result) {
			*result = ret;
		}
		return TJS_S_OK;
	}

	// �X�e�[�g�����
	void close() {
		if (stmt) {
			sqlite3_finalize(stmt);
			stmt = NULL;
		}
	}

	// sql�擾
	tTJSString getSql() const {
		ttstr ret;
		const char *sql = sqlite3_sql(stmt);
		if (sql) {
			int	len = ::MultiByteToWideChar(CP_UTF8, 0, sql, -1, NULL, 0);
			if (len > 0) {
				tjs_char *str = ret.AllocBuffer(len);
				::MultiByteToWideChar(CP_UTF8, 0, sql, -1, str, len);
			}
		}
		return ret;
	}

	// �o�C���h��Ԃ̃��Z�b�g
	int reset() {
		bindPos = 1;
		return sqlite3_reset(stmt);
	}

	// �p�����[�^�̃o�C���h
	int bind(tTJSVariant params) {
		return ::bindParams(stmt, params);
	}

	// �w��ʒu�o�C���h
	static tjs_error bindAt(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, SqliteStatement *self) {
		if (numparams < 1) {
			return TJS_E_BADPARAMCOUNT;
		}
		int ret = self->_bindAt(*params[0], numparams > 0 ? params[1] : NULL);
		if (result) {
			*result = ret;
		}
		return TJS_S_OK;
	}

	// �P�����s
	int exec() {
		int ret = sqlite3_step(stmt);
		if (ret != SQLITE_ROW) {
			reset();
		}
		return ret;
	}

	// �X�e�b�v���s
	bool step() {
		if (sqlite3_step(stmt) == SQLITE_ROW) {
			return true;
		}
		reset();
		return false;
	}

	// �f�[�^��
	int getCount() const {
		return sqlite3_data_count(stmt);
	}

	// �J������
	int getColumnCount() const {
		return sqlite3_column_count(stmt);
	}

	// �w��J������ NULL���H
	bool isNull(tTJSVariant column) const {
		return sqlite3_column_type(stmt, getColumnNo(column)) == SQLITE_NULL;
	}

	// �J�����̌^
	int getType(tTJSVariant column) const {
		return sqlite3_column_type(stmt, getColumnNo(column));
	}

	// �J������
	ttstr getName(tTJSVariant column) const {
		return (const tjs_char *)sqlite3_column_name16(stmt, getColumnNo(column));
	}

	// �l�̎擾
	static tjs_error get(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, SqliteStatement *self) {
		if (result) {
			if (numparams == 0) {
				int count = sqlite3_column_count(self->stmt);
				iTJSDispatch2 *line = TJSCreateArrayObject();
				for (int i=0;i<count;i++) {
					tTJSVariant col, *param = &col;
					::getColumnData(self->stmt, col, i);
					line->FuncCall(0, L"add", NULL, NULL, 1, &param, line);
				}
				*result = tTJSVariant(line, line);
				line->Release();
			} else {
				int col = self->getColumnNo(*params[0]);
				if (sqlite3_column_type(self->stmt, col) == SQLITE_NULL) {
					if (numparams > 1) {
						*result = *params[1];
					} else {
						result->Clear();
					}
				} else {
					::getColumnData(self->stmt, *result, col);
				}
			}
		}
		return TJS_S_OK;
	}

	// missing method
	static tjs_error missing(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, SqliteStatement *self) {
		if (numparams < 3) {return TJS_E_BADPARAMCOUNT;};
		bool ret = false;
		if (!(int)*params[0]) {
			int col = self->getColumnNo(*params[1]);
			if (col >= 0) {
				tTJSVariant value;
				::getColumnData(self->stmt, value, col);
				params[2]->AsObjectClosureNoAddRef().PropSet(0, NULL, NULL, &value, NULL);
				ret = true;
			}
		}
		if (result) {
			*result = ret;
		}
		return TJS_S_OK;
	}
	
	/**
	 * �C���X�^���X�����t�@�N�g��
	 */
	static tjs_error factory(SqliteStatement **result, tjs_int numparams, tTJSVariant **params, iTJSDispatch2 *objthis) {
		if (numparams < 1) {
			return TJS_E_BADPARAMCOUNT;
		}
		if (params[0]->AsObjectClosureNoAddRef().IsInstanceOf(0, NULL, NULL, L"Sqlite", NULL) != TJS_S_TRUE) {
			TVPThrowExceptionMessage(L"use Sqlite class Object");
		}
		SqliteStatement *state = new SqliteStatement(*params[0]);
		if (numparams > 1) {
			if (state->_open(params[1]->GetString(), numparams > 2 ? params[2] : NULL) != SQLITE_OK) {
				delete state;
				TVPThrowExceptionMessage(L"failed to open state");
				return TJS_E_FAIL;
			}
		}
		tTJSVariant name(TJS_W("missing"));
		objthis->ClassInstanceInfo(TJS_CII_SET_MISSING, 0, &name);
		*result = state;
		return TJS_S_OK;
	}
	
protected:

	/**
	 * �J�����ԍ����擾
	 * @param column �J�����w��
	 * @return �J�����ԍ�
	 */
	int getColumnNo(const tTJSVariant &column) const {
		switch (column.Type()) {
		case tvtInteger:
		case tvtReal:
			return column;
		case tvtString:
			{
				const tjs_char *col = column.GetString();
				int count = sqlite3_column_count(stmt);
				for (int i=0; i<count; i++) {
					if (_wcsicmp(col, (const tjs_char *)sqlite3_column_name16(stmt, i)) == 0)  {
						return i;
					}
				}
			}
		default:
			return -1;
		}
	}

	/**
	 * sql ���J��
	 * @param sql �X�e�[�g�Ƃ��ĊJ��sql
	 * @param params �p�����[�^
	 * @return �G���[�R�[�h
	 */
	int _open(const tjs_char *sql, const tTJSVariant *params = NULL) {
		close();
		int ret;
		if ((ret = sqlite3_prepare16_v2(db, sql, -1, &stmt, NULL)) == SQLITE_OK) {
			reset();
			if (params) {
				ret = ::bindParams(stmt, *params);
			}
		}
		return ret;
	}

	// �w��ʒu�o�C���h
	int _bindAt(tTJSVariant &value, tTJSVariant *pos=NULL) {
		if (pos) {
			bindPos = ::getBindPos(stmt, *pos);
		}
		return ::bindParam(stmt, value, bindPos++);
	}
	
private:
	tTJSVariant sqlite;
	sqlite3 *db;
	sqlite3_stmt *stmt;
	int bindPos;
};

NCB_REGISTER_CLASS(SqliteStatement) {
	ENUM(SQLITE_INTEGER);
	ENUM(SQLITE_FLOAT);
	ENUM(SQLITE_TEXT);
	ENUM(SQLITE_BLOB);
	ENUM(SQLITE_NULL);
	Factory(&ClassT::factory);
	RawCallback(TJS_W("open"), &Class::open, 0);
	NCB_METHOD(close);
	NCB_PROPERTY_RO(sql, getSql);
	NCB_METHOD(reset);
	NCB_METHOD(bind);
	RawCallback(TJS_W("bindAt"), &Class::bindAt, 0);
	NCB_METHOD(exec);
	NCB_METHOD(step);
	NCB_PROPERTY_RO(count, getCount);
	NCB_PROPERTY_RO(columnCount, getColumnCount);
	NCB_METHOD(isNull);
	NCB_METHOD(getType);
	NCB_METHOD(getName);
	RawCallback(TJS_W("get"), &Class::get, 0);
	RawCallback(TJS_W("missing"), &Class::missing, 0);
};


/**
 * Sqlite�̃X���b�h�����������N���X
 */
class SqliteThread {

public:

	// �X�e�[�g
	enum State {
		INIT,
		WORKING,
		DONE
	};

	/**
	 * �R���X�g���N�^
	 */
	SqliteThread(iTJSDispatch2 *objthis, tTJSVariant &window, tTJSVariant &sqlite)
		 : objthis(objthis), window(window), sqlite(sqlite), db(NULL), stmt(NULL), progressUpdateCount(PROGRESS_COUNT),
		   threadHandle(NULL), canceled(false), state(INIT), errorCode(0)
	{
		Sqlite *sq = ncbInstanceAdaptor<Sqlite>::GetNativeInstance(sqlite.AsObjectNoAddRef());
		if (sq) {
			db = sq->getDatabase();
		}
		setReceiver(true);
	}

	/**
	 * �f�X�g���N�^
	 */
	~SqliteThread() {
		abort();
		setReceiver(false);
	}

	// �I�������J�n
	static tjs_error select(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, SqliteThread *self) {
		if (numparams < 1) {
			return TJS_E_BADPARAMCOUNT;
		}
		self->onStateChange(INIT);
		bool ret = self->open(params[0]->GetString(), numparams > 1 ? params[1] : NULL) == SQLITE_OK;
		if (ret) {
			self->startSelectThread();
		}
		if (result) {
			*result = ret;
		}
		return TJS_S_OK;
	}

	// �X�V�����J�n
	static tjs_error update(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, SqliteThread *self) {
		if (numparams < 2) {
			return TJS_E_BADPARAMCOUNT;
		}
		self->onStateChange(INIT);
		bool ret = self->open(params[0]->GetString()) == SQLITE_OK;
		if (ret) {
			self->startUpdateThread(*params[1]);
		}
		if (result) {
			*result = ret;
		}
		return TJS_S_OK;
	}

	// �������f
	void abort() {
		stopThread();
		updateData.Clear();
		selectResult.Clear();
	}

	// �X�e�[�g�擾
	int getState() {
		return state;
	}

	// �������ʎ擾
	const tTJSVariant &getSelectResult() const {
		return selectResult;
	}
	
	// �G���[�R�[�h�擾
	int getErrorCode() {
		return errorCode;
	}

	int getProgressUpdateCount() {
		return progressUpdateCount;
	}

	void setProgressUpdateCount(int pc) {
		progressUpdateCount = pc;
	}

	/**
	 * �C���X�^���X�����t�@�N�g��
	 */
	static tjs_error factory(SqliteThread **result, tjs_int numparams, tTJSVariant **params, iTJSDispatch2 *objthis) {
		if (numparams < 2) {
			return TJS_E_BADPARAMCOUNT;
		}
		if (params[0]->AsObjectClosureNoAddRef().IsInstanceOf(0, NULL, NULL, L"Window", NULL) != TJS_S_TRUE) {
			TVPThrowExceptionMessage(L"use Window class Object");
		}
		if (params[1]->AsObjectClosureNoAddRef().IsInstanceOf(0, NULL, NULL, L"Sqlite", NULL) != TJS_S_TRUE) {
			TVPThrowExceptionMessage(L"use Sqlite class Object");
		}
		*result = new SqliteThread(objthis, *params[0], *params[1]);
		return TJS_S_OK;
	}
	
protected:

	/**
	 * sql ���J��
	 * @param sql �X�e�[�g�Ƃ��ĊJ��sql
	 * @return �G���[�R�[�h
	 */
	int open(const tjs_char *sql, const tTJSVariant *params = NULL) {
		if (threadHandle) {
			TVPThrowExceptionMessage(TJS_W("already running"));
		}
		close();
		if ((errorCode = sqlite3_prepare16_v2(db, sql, -1, &stmt, NULL)) == SQLITE_OK) {
			sqlite3_reset(stmt);
			if (params) {
				if ((errorCode = ::bindParams(stmt, *params)) != SQLITE_OK) {
					close();
				}
			}
		}
		return errorCode;
	}

	void close() {
		if (stmt) {
			sqlite3_finalize(stmt);
			stmt = NULL;
		}
	}

	// �X�e�[�g�ύX
	void onStateChange(State state) {
		if (state == DONE) {
			stopThread();
		}
		this->state = state;
		tTJSVariant param = state;
		static ttstr eventName(TJS_W("onStateChange"));
		TVPPostEvent(objthis, objthis, eventName, 0, TVP_EPT_POST, 1, &param);
	}

	// ���s�o��
	void onProgress(int n) {
		tTJSVariant param = n;
		static ttstr eventName(TJS_W("onProgress"));
		TVPPostEvent(objthis, objthis, eventName, 0, TVP_EPT_POST, 1, &param);
	}
	
	// ���[�U���b�Z�[�W���V�[�o�̓o�^/����
	void setReceiver(bool enable) {
		tTJSVariant mode     = enable ? (tTVInteger)(tjs_int)wrmRegister : (tTVInteger)(tjs_int)wrmUnregister;
		tTJSVariant proc     = (tTVInteger)(tjs_int)receiver;
		tTJSVariant userdata = (tTVInteger)(tjs_int)this;
		tTJSVariant *p[] = {&mode, &proc, &userdata};
		if (window.AsObjectClosureNoAddRef().FuncCall(0, L"registerMessageReceiver", NULL, NULL, 4, p, NULL) != TJS_S_OK) {
			TVPThrowExceptionMessage(L"can't regist user message receiver");
		}
	}

	/**
	 * �C�x���g��M����
	 */
	static bool __stdcall receiver(void *userdata, tTVPWindowMessage *Message) {
		switch (Message->Msg) {
		case WM_SQLITE_STATECHANGE: 
			{
				SqliteThread *self = (SqliteThread*)userdata;
				if (self == (SqliteThread*)Message->WParam) {
					self->onStateChange((State)Message->LParam);
					return true;
				}
			}
			break;
		case WM_SQLITE_PROGRESS:
			{
				SqliteThread *self = (SqliteThread*)userdata;
				if (self == (SqliteThread*)Message->WParam) {
					self->onProgress((int)Message->LParam);
					return true;
				}
			}
			break;
		}
		return false;
	}

	// -----------------------------------------------
	// �X���b�h����
	// -----------------------------------------------

	// ���b�Z�[�W���M
	void postMessage(UINT msg, WPARAM wparam=NULL, LPARAM lparam=NULL) {
		// �E�B���h�E�n���h�����擾���Ēʒm
		tTJSVariant val;
		window.AsObjectClosureNoAddRef().PropGet(0, TJS_W("HWND"), NULL, &val, NULL);
		HWND hwnd = reinterpret_cast<HWND>((tjs_int)(val));
		::PostMessage(hwnd, msg, wparam, lparam);
		::Sleep(0);
	}

	// -----------------------------------------------------------------------------------
	
	/**
	 * �o�b�N�O���E���h�Ŏ��s���鏈��
	 */
	void selectThreadMain() {
		postMessage(WM_SQLITE_STATECHANGE, (WPARAM)this, (LPARAM)WORKING);
		int n = 0;
		int nc = progressUpdateCount;
		tTJSVariantClosure &vc = selectResult.AsObjectClosureNoAddRef();
		while (!canceled && (errorCode = sqlite3_step(stmt)) == SQLITE_ROW) {
			int argc = sqlite3_data_count(stmt);
			iTJSDispatch2 *line = TJSCreateArrayObject();
			for (int i=0;i<argc;i++) {
				tTJSVariant value;
				::getColumnData(stmt, value, i);
				tTJSVariant *params[] = { &value };
				line->FuncCall(0, L"add", NULL, NULL, 1, params, line);
			}
			tTJSVariant l(line,line);
			line->Release();
			tTJSVariant *params[] = { &l };
			vc.FuncCall(0, L"add", NULL, NULL, 1, params, NULL);
			n++;
			if (n == nc) {
				postMessage(WM_SQLITE_PROGRESS, (WPARAM)this, (LPARAM)nc);
				nc += progressUpdateCount;
			}
		}
		if (canceled) {
			errorCode = SQLITE_ABORT;
		}
		if (errorCode == SQLITE_OK || errorCode == SQLITE_DONE) {
			postMessage(WM_SQLITE_PROGRESS, (WPARAM)this, (LPARAM)n);
		}
		postMessage(WM_SQLITE_STATECHANGE, (WPARAM)this, (LPARAM)DONE);
	}

	// ���s�X���b�h
	static unsigned __stdcall selectThreadFunc(void *data) {
		((SqliteThread*)data)->selectThreadMain();
		_endthreadex(0);
		return 0;
	}

	// �X���b�h�����J�n
	void startSelectThread() {
		iTJSDispatch2 *array = TJSCreateArrayObject();
		selectResult = tTJSVariant(array, array);
		array->Release();
		errorCode = SQLITE_OK;
		canceled = false;
		threadHandle = (HANDLE)_beginthreadex(NULL, 0, selectThreadFunc, this, 0, NULL);
	}

	// -----------------------------------------------------------------------------------

	/**
	 * �o�b�N�O���E���h�Ŏ��s���鏈��
	 */
	void updateThreadMain() {
		postMessage(WM_SQLITE_STATECHANGE, (WPARAM)this, (LPARAM)WORKING);
		int n = 0;
		int nc = progressUpdateCount;
		tTJSVariantClosure &vc = updateData.AsObjectClosureNoAddRef();
		int count = 0;
		{
			tTJSVariant cnt;
			vc.PropGet(0, L"count", NULL, &cnt, NULL);
			count = cnt;
		}
		sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
		while (!canceled && n < count && (errorCode == SQLITE_OK || errorCode == SQLITE_DONE)) {
			tTJSVariant line;
			vc.PropGetByNum(0, n++, &line, NULL);
			if ((errorCode = ::bindParams(stmt, line)) == SQLITE_OK) {
				while ((errorCode = sqlite3_step(stmt)) == SQLITE_ROW);
				sqlite3_reset(stmt);
			}
			if (n == nc) {
				postMessage(WM_SQLITE_PROGRESS, (WPARAM)this, (LPARAM)n);
				nc += progressUpdateCount;
			}
		}
		updateData.Clear();
		if (canceled) {
			errorCode = SQLITE_ABORT;
		}
		if (errorCode == SQLITE_OK || errorCode == SQLITE_DONE) {
			sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
			postMessage(WM_SQLITE_PROGRESS, (WPARAM)this, (LPARAM)n);
		} else {
			sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
		}
		postMessage(WM_SQLITE_STATECHANGE, (WPARAM)this, (LPARAM)DONE);
	}

	// ���s�X���b�h
	static unsigned __stdcall updateThreadFunc(void *data) {
		((SqliteThread*)data)->updateThreadMain();
		_endthreadex(0);
		return 0;
	}

	// �X���b�h�����J�n
	void startUpdateThread(tTJSVariant &data) {
		updateData = data;
		errorCode = SQLITE_OK;
		canceled = false;
		threadHandle = (HANDLE)_beginthreadex(NULL, 0, updateThreadFunc, this, 0, NULL);
	}

	// -----------------------------------------------------------------------------------
	
	// �X���b�h�����I��
	void stopThread() {
		if (threadHandle) {
			canceled = true;
			WaitForSingleObject(threadHandle, INFINITE);
			CloseHandle(threadHandle);
			threadHandle = 0;
		}
		close();
	}

	
private:
	iTJSDispatch2 *objthis; ///< ���ȃI�u�W�F�N�g���̎Q��
	tTJSVariant window;
	tTJSVariant sqlite;
	sqlite3 *db;
	sqlite3_stmt *stmt;
	int progressUpdateCount;
	
	// �X���b�h�����p
	HANDLE threadHandle; ///< �X���b�h�̃n���h��
	bool canceled; ///< �L�����Z�����ꂽ
	
	tTJSVariant updateData; ///< UPDATE�p�f�[�^(�z��)
	State state; ///< �X�e�[�g
	int errorCode; ///< �G���[�R�[�h
	tTJSVariant selectResult; ///< SELECT�̌���(�z��)
};

#define ENUM2(n) Variant(#n, (int)SqliteThread::n)

NCB_REGISTER_CLASS(SqliteThread) {
	ENUM2(INIT);
	ENUM2(WORKING);
	ENUM2(DONE);
	Factory(&ClassT::factory);
	RawCallback(TJS_W("select"), &Class::select, 0);
	RawCallback(TJS_W("update"), &Class::update, 0);
	NCB_METHOD(abort);
	NCB_PROPERTY_RO(state, getState);
	NCB_PROPERTY_RO(errorCode, getErrorCode);
	NCB_PROPERTY_RO(selectResult, getSelectResult);
	NCB_PROPERTY(progressUpdateCount, getProgressUpdateCount, setProgressUpdateCount);
};

// --------------------------------------------------------------------------

extern void initNormalize();

static void
initSqlite()
{
	sqlite3_vfs_register(getXp3Vfs(), 0);
	initNormalize();
}

static void
doneSqlite()
{
	sqlite3_vfs_unregister(getXp3Vfs());
}

NCB_PRE_REGIST_CALLBACK(initSqlite);
NCB_POST_UNREGIST_CALLBACK(doneSqlite);
