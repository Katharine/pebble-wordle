FROM golang:1.16.3 as build

WORKDIR /go/src/app

ADD . /go/src/app
RUN CGO_ENABLED=0 GOOS=linux go build -o /go/bin/app

# This app is so simple it can happily run in an empty filesystem.
FROM scratch
COPY --from=build /go/bin/app /
ENTRYPOINT ["/app"]
