package networkproject;

import ned.IdealChannel;



@license(LGPL);
//
// TODO documentation
//
network Network
{
    @display("bgb=843,375");
    submodules:
        node: Node {
            @display("p=243,101");
        }
        node1: Node {
            @display("p=436,89");
        }
        coordinator: Coordinator {
            @display("p=337,196");
        }
    connections:
        coordinator.out[1] --> IdealChannel --> node1.in[0];
        coordinator.out[0] --> IdealChannel --> node.in[0];
        node.out --> IdealChannel --> node1.in[1];
        node1.out --> IdealChannel --> node.in[1];
}

