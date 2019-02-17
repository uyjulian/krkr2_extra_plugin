//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
#include <windows.h>
#include "tp_stub.h"

//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
// テストクラス
//---------------------------------------------------------------------------
/*
	各オブジェクト (iTJSDispatch2 インターフェース) にはネイティブインスタンスと
	呼ばれる、iTJSNativeInstance 型のオブジェクトを登録することができ、これを
	オブジェクトから取り出すことができます。
	まず、ネイティブインスタンスの実装です。ネイティブインスタンスを実装するには
	tTJSNativeInstance からクラスを導出します。tTJSNativeInstance は
	iTJSNativeInstance の基本的な動作を実装しています。
*/
class NI_Parser : public tTJSNativeInstance // ネイティブインスタンス
{
public:
	NI_Parser()
	{
		// コンストラクタ
		/*
			NI_Parser のコンストラクタです。C++ クラスとしての初期化は 後述の
			Construct よりもここで済ませておき、Construct での初期化は最小限の物
			にすることをおすすめします。
		*/
		DicObj = NULL;
		Dictionary_clear = NULL;

		/*
			Name_ch などにあらかじめ 文字列 を入れておく
			ttstr は、それが保持している文字列の ハッシュ もともに保存すること
			ができるため、文字列の検索の効率をよくするために
			あらかじめ文字列を設定し、この文字列を iTJSDispatch2::PropSet などで
			使い回す
		*/
		Name_ch = TJS_W("ch"); // Name_ch

		try
		{
			// 辞書配列オブジェクトを作成する
			// 辞書配列オブジェクトは、毎回の getNext 呼び出しで再生成
			// されないように、一回再生した物を使い回すこととする
			DicObj = TJSCreateDictionaryObject();

			// Dictionary.clear を取得する
			// これも、毎回 Dictionary.clear を取得していると時間がかかるため
			// ここであらかじめ取得しておく
			tTJSVariant val;
			TVPExecuteExpression(TJS_W("global.Dictionary.clear"), &val);
			Dictionary_clear = val.AsObject();
				// AsObject は参照カウントを増やすので
				// あとで Dictionary_clear を Release する必要あり
		}
		catch(...)
		{
			// 上記 try ブロック中で例外が発生した場合の保護
			Invalidate();
			throw;
		}
	}

	tjs_error TJS_INTF_METHOD
		Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj)
	{
		// TJS2 オブジェクトが作成されるときに呼ばれる
		/*
			TJS2 の new 演算子で TJS2 オブジェクトが作成されるときに呼ばれます。
			numparams と param 引数は new 演算子に渡された引数を表しています。
			tjs_obj 引数は、作成される TJS オブジェクトです。
		*/

		// 引数があればそれを初期値として Value に入れる
		return S_OK;
	}

	void TJS_INTF_METHOD Invalidate()
	{
		// オブジェクトが無効化されるときに呼ばれる
		/*
			オブジェクトが無効化されるときに呼ばれるメソッドです。ここに終了処理
			を書くと良いでしょう。
		*/
		if(DicObj) DicObj->Release();
		if(Dictionary_clear) Dictionary_clear->Release();
	}

	/*
		データメンバを操作するための公開メソッド群です。後述するネイティブクラス
		内で、これらを利用するコードを書きます。
	*/

	void Load(const ttstr & name)
	{
		// Script にスクリプトを読み込みます
		// TextReadStream を作成し、それを利用してスクリプトを読み込みます
		iTJSTextReadStream * stream =
			TVPCreateTextStreamForRead(name, TJS_W(""));
		try
		{
			stream->Read(Script, 0); // 全てを一気に読み込み
			Pointer = 0; // ポインタの初期化
		}
		catch(...)
		{
			stream->Destruct();
			throw;
		}
		stream->Destruct();
	}

	iTJSDispatch2 * GetNext()
	{
		// 次のトークンの情報を DicObj に入れる
		// 読み出しに成功すれば DicObj オブジェクト、
		// 失敗すれば (スクリプトの終端に達すれば) NULL
		// を返す。
		// 返される DicObj オブジェクトの参照カウンタは
		// インクリメント *されない* ので注意

		// DicObj をクリア
		Dictionary_clear->FuncCall(
			0, // flag
			NULL, // membername
			NULL, // hint
			NULL, // result
			0, // numparams
			NULL, // param
			DicObj // objthis
			);

		// スクリプトの終端に達したか?
		// note: ttstr は内部に文字列の長さを持っているので
		// strlen のように文字列長をスキャンして毎回得る事はないので
		// 比較的高速
		if(Pointer >= Script.GetLen()) return NULL;

		// 一文字を得て、DicObj の ch メンバに設定する
		// また、Pointer をインクリメントする
		// note: ここでは簡単のために１コードポイント＝１文字であると見なす
		tjs_char ch = Script.c_str()[Pointer];
		Pointer ++;

		ttstr str (ch); // ch を文字列に変換
		tTJSVariant val(str); // str を tTJSVariant に変換
		DicObj->PropSet(
			TJS_MEMBERENSURE, // flags: メンバが無ければ作成することを強制する
			Name_ch.c_str(), // 設定するメンバ名
			Name_ch.GetHint(), // 設定するメンバ名のヒント
			&val, // 設定する Variant
			DicObj // objthis: コンテキスト
			);

		// DicObj を返す
		// 参照カウンタのインクリメントは行わない
		return DicObj;
	}

private:
	/*
		保護メソッドなど
	*/

private:
	/*
		データメンバです。ネイティブインスタンスには、必要なデータメンバを自由に
		書くことができます。
	*/
	iTJSDispatch2 * DicObj; // getNext 関数で毎回返される辞書配列オブジェクト
	iTJSDispatch2 * Dictionary_clear; // Dictionary.clear 関数

	ttstr Script; // 入力スクリプトを保持するための文字列
	tjs_int Pointer; // 解析位置

	ttstr Name_ch; // "ch" を保持する文字列
};
//---------------------------------------------------------------------------
/*
	これは NI_Parser のオブジェクトを作成して返すだけの関数です。
	後述の TJSCreateNativeClassForPlugin の引数として渡します。
*/
static iTJSNativeInstance * TJS_INTF_METHOD Create_NI_Parser()
{
	return new NI_Parser();
}
//---------------------------------------------------------------------------
/*
	TJS2 のネイティブクラスは一意な ID で区別されている必要があります。
	これは後述の TJS_BEGIN_NATIVE_MEMBERS マクロで自動的に取得されますが、
	その ID を格納する変数名と、その変数をここで宣言します。
	初期値には無効な ID を表す -1 を指定してください。
*/
#define TJS_NATIVE_CLASSID_NAME ClassID_Parser
static tjs_int32 TJS_NATIVE_CLASSID_NAME = -1;
//---------------------------------------------------------------------------
/*
	TJS2 用の「クラス」を作成して返す関数です。
*/
static iTJSDispatch2 * Create_NC_Parser()
{
	/*
		まず、クラスのベースとなるクラスオブジェクトを作成します。
		これには TJSCreateNativeClassForPlugin を用います。
		TJSCreateNativeClassForPlugin の第１引数はクラス名、第２引数は
		ネイティブインスタンスを返す関数を指定します。
		作成したオブジェクトを一時的に格納するローカル変数の名前は
		classobj である必要があります。
	*/
	tTJSNativeClassForPlugin * classobj =
		TJSCreateNativeClassForPlugin(TJS_W("Parser"), Create_NI_Parser);


	/*
		TJS_BEGIN_NATIVE_MEMBERS マクロです。引数には TJS2 内で使用するクラス名
		を指定します。
		このマクロと TJS_END_NATIVE_MEMBERS マクロで挟まれた場所に、クラスの
		メンバとなるべきメソッドやプロパティの記述をします。
	*/
	TJS_BEGIN_NATIVE_MEMBERS(/*TJS class name*/Parser)

		/*
			空の finalize メソッドを宣言します。finalize に相当する処理は
			tTJSNativeInstance::Invalidate をオーバーライドすることでも実装でき
			ますので、通常は空のメソッドで十分です。
		*/
		TJS_DECL_EMPTY_FINALIZE_METHOD

		/*
			(TJSの) コンストラクタを宣言します。TJS でクラスを書くとき、
			クラス内でクラスと同名のメソッドを宣言している部分に相当します。

			TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL マクロの１番目の引数はネイティブ
			インスタンスに割り当てる変数名で、２場面目の引数はその変数の型名です。
			このブロック内では NI_Parser * _this という変数が利用可能で、
			ネイティブインスタンスにアクセスすることができます。
			マクロの３番目の引数は、TJS 内で使用するクラス名を指定します。
			TJS_END_NATIVE_CONSTRUCTOR_DECL マクロの引数も同様です。
			ここも、コンストラクタに相当する処理は tTJSNativeInstance::Construct
			をオーバーライドする事で実装できるので、ここでは何もせずに S_OK を返
			します。
		*/
		TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(
			/*var.name*/_this,
			/*var.type*/NI_Parser,
			/*TJS class name*/Parser)
		{
			// NI_Parser::Construct にも内容を記述できるので
			// ここでは何もしない
			return TJS_S_OK;
		}
		TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/Parser)

		/*
			print メソッドを宣言します。メソッド名は
			TJS_BEGIN_NATIVE_METHOD_DECL と TJS_END_NATIVE_METHOD_DECL の両マク
			ロに同じものを指定する必要があります。このマクロ内で使用可能な変数に
			tjs_int numparams と tTJSVariant **param があって、それぞれ、渡され
			た引数の数と引数を示しています。このメソッドではそれらは使用していま
			せん。TJS_GET_NATIVE_INSTANCEは、オブジェクトからネイティブインスタン
			スを取り出すためのマクロです。ここでは、_this という NI_Parser *
			型の変数にネイティブインスタンスを取り出す、という意味になります。
			以降、_this という変数でネイティブインスタンスにアクセスできます。
		*/
		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/load) // load メソッド
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
				/*var. type*/NI_Parser);

			if(numparams < 1) return TJS_E_BADPARAMCOUNT; // 引数の個数チェック

			_this->Load(*param[0]);

			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/load)


		/*
			getNext メソッド
		*/
		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/getNext) // getNext メソッド
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
				/*var. type*/NI_Parser);

			iTJSDispatch2 * obj = _this->GetNext();

			if(result) *result = obj; // result は null になり得るので注意

			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/getNext)

	TJS_END_NATIVE_MEMBERS

	/*
		この関数は classobj を返します。
	*/
	return classobj;
}
//---------------------------------------------------------------------------
/*
	TJS_NATIVE_CLASSID_NAME は一応 undef しておいたほうがよいでしょう
*/
#undef TJS_NATIVE_CLASSID_NAME
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason,
	void* lpReserved)
{
	return 1;
}
//---------------------------------------------------------------------------
static tjs_int GlobalRefCountAtInit = 0;
extern "C" HRESULT _stdcall _export V2Link(iTVPFunctionExporter *exporter)
{
	// スタブの初期化(必ず記述する)
	TVPInitImportStub(exporter);

	tTJSVariant val;

	// TJS のグローバルオブジェクトを取得する
	iTJSDispatch2 * global = TVPGetScriptDispatch();


	//-----------------------------------------------------------------------
	// 1 まずクラスオブジェクトを作成
	iTJSDispatch2 * tjsclass = Create_NC_Parser();

	// 2 tjsclass を tTJSVariant 型に変換
	val = tTJSVariant(tjsclass);

	// 3 すでに val が tjsclass を保持しているので、tjsclass は
	//   Release する
	tjsclass->Release();


	// 4 global の PropSet メソッドを用い、オブジェクトを登録する
	global->PropSet(
		TJS_MEMBERENSURE, // メンバがなかった場合には作成するようにするフラグ
		TJS_W("Parser"), // メンバ名 ( かならず TJS_W( ) で囲む )
		NULL, // ヒント ( 本来はメンバ名のハッシュ値だが、NULL でもよい )
		&val, // 登録する値
		global // コンテキスト ( global でよい )
		);
	//-----------------------------------------------------------------------


	// - global を Release する
	global->Release();

	// もし、登録する関数が複数ある場合は 1 〜 4 を繰り返す


	// val をクリアする。
	// これは必ず行う。そうしないと val が保持しているオブジェクト
	// が Release されず、次に使う TVPPluginGlobalRefCount が正確にならない。
	val.Clear();


	// この時点での TVPPluginGlobalRefCount の値を
	GlobalRefCountAtInit = TVPPluginGlobalRefCount;
	// として控えておく。TVPPluginGlobalRefCount はこのプラグイン内で
	// 管理されている tTJSDispatch 派生オブジェクトの参照カウンタの総計で、
	// 解放時にはこれと同じか、これよりも少なくなってないとならない。
	// そうなってなければ、どこか別のところで関数などが参照されていて、
	// プラグインは解放できないと言うことになる。

	return S_OK;
}
//---------------------------------------------------------------------------
extern "C" HRESULT _stdcall _export V2Unlink()
{
	// 吉里吉里側から、プラグインを解放しようとするときに呼ばれる関数。

	// もし何らかの条件でプラグインを解放できない場合は
	// この時点で E_FAIL を返すようにする。
	// ここでは、TVPPluginGlobalRefCount が GlobalRefCountAtInit よりも
	// 大きくなっていれば失敗ということにする。
	if(TVPPluginGlobalRefCount > GlobalRefCountAtInit) return E_FAIL;
		// E_FAIL が帰ると、Plugins.unlink メソッドは偽を返す


	/*
		ただし、クラスの場合、厳密に「オブジェクトが使用中である」ということを
		知るすべがありません。基本的には、Plugins.unlink によるプラグインの解放は
		危険であると考えてください (いったん Plugins.link でリンクしたら、最後ま
		でプラグインを解放せず、プログラム終了と同時に自動的に解放させるのが吉)。
	*/

	// TJS のグローバルオブジェクトに登録した Parser クラスなどを削除する

	// - まず、TJS のグローバルオブジェクトを取得する
	iTJSDispatch2 * global = TVPGetScriptDispatch();

	// - global の DeleteMember メソッドを用い、オブジェクトを削除する
	if(global)
	{
		// TJS 自体が既に解放されていたときなどは
		// global は NULL になり得るので global が NULL でない
		// ことをチェックする

		global->DeleteMember(
			0, // フラグ ( 0 でよい )
			TJS_W("Parser"), // メンバ名
			NULL, // ヒント
			global // コンテキスト
			);
	}

	// - global を Release する
	if(global) global->Release();

	// スタブの使用終了(必ず記述する)
	TVPUninitImportStub();

	return S_OK;
}
//---------------------------------------------------------------------------

