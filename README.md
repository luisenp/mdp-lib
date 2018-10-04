mdp-lib
=======

A Library of algorithms for planning using Markov Decision Procceses. 

The library provides abstractions for representing MDPs and implementations of several popular MDP solvers, such as Value Iteration, LAO*, LRTDP and HDP. It also supports problems described in the <a href="http://www.tempastic.org/papers/CMU-CS-04-167.pdf">PPDDL format</a>. For these, a variety of determinization-based solvers are provided, such as FF-Replan, RFF and FF-LAO*.

The library has been tested in Ubuntu 12.04 and 14.04 (64-bit).

<b>Some compilation notes</b>

<ul>
  <li>The PPDDL readed uses code from <a href="https://code.google.com/archive/p/mini-gpt/">mini-gpt</a>. To compile mini-gpt you will need the following dependencies:
  <ul>
    <li>yacc -- In Ubuntu you can run <code>sudo apt-get install bison</code>. </li>
    <li>flex -- In Ubuntu you can run <code>sudo apt-get install flex</code>. </li>
  </ul>
</ul>
  
