CXX = g++
CPPFLAGS += -I/usr/local/include -pthread
CXXFLAGS += -std=c++11
ifeq ($(SYSTEM),Darwin)
LDFLAGS += -L/usr/local/lib `pkg-config --libs grpc++ grpc`       \
           -lgrpc++_reflection \
           -lprotobuf -lpthread -ldl
else
LDFLAGS += -L/usr/local/lib `pkg-config --libs grpc++ grpc`       \
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed \
           -lprotobuf -lpthread -ldl
endif


all:greeter_client

greeter_client: ../iShare_source/iShare.pb.o ../iShare_source/iShare.grpc.pb.o greeter_client.o
	$(CXX) $^ $(LDFLAGS) -o $@
clean:
	rm -f *.o *.pb.cc *.pb.h greeter_client greeter_server greeter_async_client greeter_async_client2 greeter_async_server


