all:
	@echo  \ "build - собрать\n" \
		"clear - очистка\n"\
		"Запустить: sudo ./miniTron <длина экрана> <ширина экрана> <ip:порт соперника>\n"
build:
	@gcc -c *.c
	@gcc -o miniTron.exe *.o -lpthread
clear:
	@rm *.exe *.o
