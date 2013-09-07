# OpenCV + Oculus Rift
두개의 카메라의 입력을 그대로 Oculus Rift로 보내는 프로그램이다. 캡쳐한 이미지를 후처리하는 것을 구현까지 구현했다.

![no effect](https://raw.github.com/shipduck/haruna/master/document/no_effect.jpg)
![grayscale](https://raw.github.com/shipduck/haruna/master/document/gray_scale.jpg)
![sepia](https://raw.github.com/shipduck/haruna/master/document/sepia.jpg)
![emboss](https://raw.github.com/shipduck/haruna/master/document/emboss.jpg)
![edge detedtion](https://raw.github.com/shipduck/haruna/master/document/edge_detect.jpg)
![물체 배치](https://raw.github.com/shipduck/haruna/master/document/object_pos.jpg)

## 실행 방법
웹캠을 하나 이상 연결한 상태에서 실행을 적절히 한다. 콘솔에서 f나 w를 선택한다(f=fullscreen, w=window mode). 
실행중에 1/2/3/4/5를 누르면 후처리 효과를 바꾼다. (1=no effect, 2=gray scale, 3=sepia, 4=emboss, 5=edge detect)

## 결과
웹캠를 싼거로 사거 그런지 기본 FPS가 매우 떨어졌다. -_-...좋은 웹캠를 빌리거나 현질을 하면 다시 테스트 해봐야겠다.

내가 사용한 웹캠은 640x480 30fps였다. 이정도 해상도로는 오큘러스에서 엄청나게 뭉개진다. 
그렇다고 작정하고 고해상도 웹캠을 2개 박으면 해상도 문제는 해결되겠지만 대역폭에서 문제 생길 가능성이 높다.
어느정도 수준의 해상도를 사용해야 대역폭 문제가 없이 잘 돌아가는지 테스트 하기위해서는 역시 좋은 웹캠이 있어야겠다. -_-

후처리를 달았는데 프레임이 엄청 떨어지더라. 이건 내 컴터가 후져서 그런거같다. 컴터도 바꿔야하나....-_-

그냥 3D programming에서는 별 신경을 쓰지 않았는데 webcam을 이용하니까 
projection matrix를 내가 통제할수 없어서 거리감(?)을 잡기가 어렵더라.
예제 스샷을 찍기 위해서 가까이에 물체를 배치했더니 카메라를 가득 차지해버려 일부러 좀 멀리 배치했다. 

카메라 2개를 고정할수 있는 장치가 따로 없어서 초점맞추는거 어려웠다. 
원래는 모자같은곳에 카메라 2개를 장착하려고 했으나 포기함.

GrayScale, Sepia 색 변환은 그 결과물을 봐도 입체감이 느껴지지만 emboss, edge detect은 입체감이 느껴지지 않는다. 
오히려 볼수록 뭔가 이상하다. 아마도 현실에는 존재할수 없는 렌더링 결과물을 양쪽눈으로 보니까 제대로 인식할수 없는거같다.
**edge detection이 어색한거로 추측하건데 Oculus Rift 대응 게임에서 외곽선 효과를 주면 어색할거같다**

## OpenCV setting
윈도우 환경 기준으로 설명한다. (내가 그거밖에 안쓰니까)

1. opencv를 받아서 적절한곳에 압축을 푼다
2. 루트 권한으로 cmd 실행후
```sh
setx -m OpenCV_DIR d:\opencv\build\x86\vc11
```
3. 환경변수 PATH에 ```%OPENCV_DIR%\bin```를 추가한다
3. ```cmake .``` 했는데 에러가 발생한 경우 ```cmake . -DOpenCV_DIR=d:\opencv```를 해본다. 안되면 삽질은 셀프

