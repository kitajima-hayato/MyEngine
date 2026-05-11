// メモリリーク検出チェック
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "Game/Scene//MyGame.h"
//ウィンドウズアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

#ifdef _DEBUG
	// デバッグモードのときメモリリーク検出を有効にする
	// メモリリーク検出
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// リーク番号を指定してブレークを発生させる
	//_CrtSetBreakAlloc(367743);

	D3DResourceLeakChecker leakChecker;
#endif
	{
		MyGame game;
		game.Run();
	}

	return 0;
}
