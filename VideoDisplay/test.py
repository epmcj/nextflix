import cv2

frameRate = 1000/24; #in milisseconds

vidcap = cv2.VideoCapture('../assets/sample.mp4')
cv2.namedWindow("Nextflix")

while True:
	success,image = vidcap.read()
	if not success:
		break
	cv2.imshow("Nextflix", image)
	if cv2.waitKey(frameRate) != -1:
		break
	
cv2.destroyWindow("Nextflix")
