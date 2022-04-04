#pragma once
#include "CameraHandler.h"

using namespace std;



int main(int argc, char** argv)
{
    /*
    * ТАК !
    * Всё работает, но я не имею возможности проверить, наскольно правильно оно работает
    * замеры времени есть при заапуске и файле логов.
    * 30 кадров вывозит, но мне сильно кажется, что это ограничение со стороны камеры, 
    * тк переход с дебага в релиз не даёт прироста в производительности (а прежде он был значителен)
    * и трансформация занимает 4 мс, а среднее время на обработку картинки 33 мс
    * его (ограничение) можно программно снять но задачи такой не было
    */

    cout << "Start work\n";
    bool work = true;

    std::thread _thread([&work]()
        {
            CameraHandler cam;
            cam.AcquireImage(work);
        }
    );
    getchar();
    work = false;
    _thread.join();
    cout << "\nEnd work\n";

    return 0;
}
