# Networking and authority

## Rule of thumb

The server owns game state. Clients may present input, prediction, and UI, but
they must not independently create authoritative enemies or apply durable combat
state.

## Enemy spawning

`UAbilityTask_WaitSpawnEnemies` explicitly refuses to spawn actors when its world
net mode is `NM_Client`. Trigger the owning ability on the server, or route client
intent through the ability/replication flow that activates it on the server.

The task also accepts only its first gameplay event. Repeated events do not create
additional batches while the soft class is loading.

## Combat and projectiles

- Give projectiles a valid instigator before enabling movement or collision.
- Apply Gameplay Effects through the authoritative combat path.
- Both projectile collision policies (`OnHit` and `OnBeginOverlap`) now use the
  same team, block, damage, hit-react, and destruction logic.
- Team checks return neutral for actors that are not possessed team pawns. Do not
  treat neutral as a hostile target without an explicit gameplay rule.

## Testing multiplayer changes

1. Run PIE with a listen server and at least one client.
2. Trigger enemy spawning from each relevant player path.
3. Confirm enemies exist once, on the server and all clients.
4. Verify projectiles deal damage once and do not damage allies.
5. Verify despawns, deaths, and UI state agree across clients.

This project does not document a full replicated ability-prediction policy yet.
Document any new RPC, replication property, or prediction decision next to the
feature that introduces it.
