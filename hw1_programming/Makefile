# main:
# 	gcc client.c -o client
# 	gcc server.c -o server

# clean:
# 	rm -f client server BulletinBoard

main:
	gcc client.c -o client
	gcc server.c -o server
	gcc lock.c -o lock
	touch BulletinBoard
	cat sampletestcases/testcase1-2-init > BulletinBoard
	./lock &

clean:
	rm -f client server BulletinBoard lock

