## Usage
```
docker build -t web-viewer -f Dockerfile.amd64 .
or
docker build -t web-viewer -f Dockerfile.armhf .
```
```
docker run --rm --net=host web-viewer --cid=214
```

Now, simply point your web-browser to the IP address and port 5011 where you
started this microservice to see any currently exchanged messages:

## License

* This project is released under the terms of the BSD-3-Clause License

