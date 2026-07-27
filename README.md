# rev-finder

To answer the question of "Erm aren't you just checking all possible seeds? What's cool about this?":

Let's say I'm searching for a village with the most torches (lamps were a thing pre-1.11) in the game. The obvious way to do this is to search all 2^32 seeds, and let's say I want this to be in the first village region, so I have to call `isVillageChunk` 28x28 = 784 times per seed. And how am I supposed to know I'll get the best result? What if there's a better village 10k blocks from spawn on some seed? Instead of doing that unsatisfying iffy search, we can use quirks in how the game works to make this 200x faster, and achieve similar results or similar searches!

The main trick is that there are only 2^32 possible RNG states a village/stronghold can have when it starts generating, and this number determines the structure's layout. I'll refer to this integer as the structure's ID. Instead of actually checking villages that exist, I start by checking all the 2^32 possible village layouts (1 per ID). Then, once I find an ID I like, I can use two more tricks to help me out:
- Each ID can only spawn in a certain position relative to the region it's in. (No more checking 784 chunks!)
- Village IDs are determined with a function that takes in the seed and chunk. It's possible to reverse this function to find the seed that would output a given ID in a certain chunk.

So anyway, say I figure out that the village ID 123 has a desirable number of torches. Then I figure out that it can only spawn at the chunk (10, 20) in the first village region. I find what seed will have this village ID at this exact chunk, and then all I have to do is check biomes (and it's good that this is the last check, since biome math is pretty slow). If the biome check passes, I've found my seed! If not, I can go to the (10, 20) chunk within the next village region, calculate the corresponding seed for that, and check that one too. I keep doing this in a spiral pattern (see `VillageIter` in utils) until I get a seed that works, at which point I've not only found an actual village with ID 123 but also verified that no closer villages exist in the game.

And once I do this for all possible village IDs, I can also be sure that I haven't missed anything! For example, I can tell you with certainty that you'll never find a village with over 30 lamps, or that you'll never find a stronghold that reaches y=73. Finding structures in reverse, hence the name of this repo, also is like hundreds of times faster, so that's a bonus too.

### reduction?

I'm learning about Lagrange-Gauss reduction (basically the 2D subset of LLL reduction) to find chunks with certain stronghold seeds faster. It's a pretty cool application of linear algebra (a field I need to learn more about, TBH)

### credit

For Cubiomes I'm using an old version of https://github.com/FragrantResult186/cubiomes-bedrock, slightly modified to add stuff like accessing torches/paths in villages or generating hypothetical versions of vllages/strongholds, assuming the global RNG had a certain value beforehand.

Remember to `git clone --recursive`!
