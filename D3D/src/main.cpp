#include "App.h"

/// <summary>
/// ���C���G���g���[�|�C���g
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
/// <param name="evnp"></param>
/// <returns></returns>
int wmain(int argc, wchar_t argv, wchar_t** evnp)
{
	// �v���O�����I�����̃��������[�N�`�F�b�N�A�o�͂��s��
#if defined(DEBUG) || defined(_DEBUG)
	{
		_CrtSetDbgFlag(
			_CRTDBG_ALLOC_MEM_DF    // �f�o�b�O�p�̃q�[�v���������g����悤�ɂȂ�
			| _CRTDBG_LEAK_CHECK_DF // �v���O�����I�����Ƀ��������[�N�`�F�b�N���s��
		);
	}
#endif
	App app(960, 540);
	app.Run();

	return 0;
}