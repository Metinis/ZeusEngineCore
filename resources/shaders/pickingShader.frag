#version 410 core

out uint outID;
uniform uint u_EntityID;

void main()
{
    outID = u_EntityID;
}