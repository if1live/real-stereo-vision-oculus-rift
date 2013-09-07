# haruna
OpenCV + Oculus Rift

# OpenCV setting
윈도우 환경 기준으로 설명한다. (내가 그거밖에 안쓰니까)

1. opencv를 받아서 적절한곳에 압축을 푼다
2. 루트 권한으로 cmd 실행후
```sh
setx -m OpenCV_DIR d:\opencv\build\x86\vc11
```
3. 환경변수 PATH에 ```%OPENCV_DIR%\bin```를 추가한다
3. ```cmake .``` 했는데 에러가 발생한 경우 ```cmake . -DOpenCV_DIR=d:\opencv```를 해본다. 안되면 삽질은 셀프

