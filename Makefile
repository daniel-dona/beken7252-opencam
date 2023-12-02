DOCKER_IMAGE = bk7252-builder


image:
	docker build -t $(DOCKER_IMAGE) .

build:
	docker run --rm -u $(shell id -u) -v$(shell pwd)/project:/project $(DOCKER_IMAGE) "scons -j$(shell nproc --all)"

sh:
	docker run --rm -u $(shell id -u) -ti -v$(shell pwd)/project:/project $(DOCKER_IMAGE) sh

clean:
	docker run --rm -v$(shell pwd)/project:/project $(DOCKER_IMAGE) "scons -c"
