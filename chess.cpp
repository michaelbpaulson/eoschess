#include <chess.hpp>
/**
 *  The init() and apply() methods must have C calling convention so that the blockchain can lookup and
 *  call these methods.
 */

void db_array_to_board(uint8_t (&board)[8][8], uint8_t (&db_array)[64]) {
  uint8_t b = 0;
  uint8_t g = 0;
  for (uint8_t i = 0; i < 64; i++) {
    board[g][b] = db_array[i];
    if (b == 7) {
      g++;
      b = 0;
    } else {
      b++;
    }
  }
}

uint8_t get_piece_type(uint8_t piece) {
  uint8_t r;
  switch ( piece ) {
  case  11 ://king
  case  31 :
    r = 0;
    break;
  case  12 ://queen
  case  32 :
    r = 1;
    break;
  case  13 ://rook ♖
  case  14 :
  case  33 :
  case  34 :
    r = 2;
    break;
  case  15 ://bishops ♗
  case  16 :
  case  35 :
  case  36 :
    r = 3;
    break;
  case  17 ://knights ♘
  case  18 :
  case  37 :
  case  38 :
    r = 4;
    break;
  case  19 ://pawns ♙
  case  20 :
  case  21 :
  case  22 :
  case  23 :
  case  24 :
  case  25 :
  case  26 :
  case  39 :
  case  40 :
  case  41 :
  case  42 :
  case  43 :
  case  44 :
  case  45 :
  case  46 :
    r = 5;
    break;
  }
  return r;
}

void add_piece_config(uint8_t (&config_array)[6], const uint8_t (&piece_config)[6]) {
  for (uint8_t g = 0; g < 6; g++) {
    config_array[g] = piece_config[g];
  }
  uint8_t x = piece_config[0];
  (x > 10 && x < 27) ? x = 0 : x = 1;
  config_array[0] = x;
}

uint8_t piece_side(uint8_t piece) {
  if (piece != 0) {
    uint8_t x;
    (piece > 10 && piece < 27) ? x = 0 : x = 1;
    return x;
  } else {
    return 100;
  }
}

void detect_check(uint8_t (&board)[8][8], uint8_t king_pos_vert, uint8_t king_pos_hor, bool is_checked) {
  uint8_t king = board[king_pos_vert][king_pos_hor];
  uint8_t kingside = piece_side(king);
  uint8_t vert_plus_one = king_pos_vert + 1;
  uint8_t vert_minus_one = king_pos_vert - 1;
  uint8_t hor_plus_one = king_pos_hor + 1;
  uint8_t hor_minus_one = king_pos_hor - 1;
  uint8_t directions [8][2] = {
    {vert_plus_one, king_pos_hor}, //vert_top
    {vert_minus_one, king_pos_hor}, //vert_bottom
    {king_pos_vert, hor_plus_one}, //hor_right
    {king_pos_vert, hor_minus_one}, //hor_left
    {vert_plus_one, hor_plus_one}, //diag_top_right
    {vert_plus_one, hor_minus_one}, //diag_top_left
    {vert_minus_one, hor_plus_one}, //diag_bottom_right
    {vert_minus_one, hor_minus_one} //diag_bottom_left
  };
  uint8_t hor, vert, p_side, direction;
  for (uint8_t j = 0; j < 8; j++) {
    vert = directions[j][0];
    hor = directions[j][1];
    for (uint8_t jj = 0; jj < 7; jj++) {
      p_side = piece_side(board[vert][hor]);
      if (vert < 0 || vert > 7 || hor < 0 || hor > 7) {
        break;
      }
      printi(vert);
      eos::print("\n");
      printi(hor);
      eos::print("\n");
      if (p_side == 100) {
        if (vert != king_pos_vert ) {
          (vert > king_pos_vert) ? vert++ : vert--;
        }
        if (hor != king_pos_hor) {
          (hor > king_pos_hor) ? hor++ : hor--;
        }
        continue;
      }
      else if (p_side != kingside) {
        switch (get_piece_type(board[vert][hor])) {
        case 0:
          (!jj) ? is_checked = true : is_checked = false;
          break;
        case 1:
          is_checked = true;
          break;
        case 2:
          (j < 4) ? is_checked = true : is_checked = false;
          break;
        case 3:
          (j > 3) ? is_checked = true : is_checked = false;
          break;
        case 4:
          is_checked = false;
          break;
        case 5:
          if (!kingside ) {
            (!jj && j > 3 && j < 6) ? is_checked = true : is_checked = false;
          }
          else {
            (!jj && j > 5) ? is_checked = true : is_checked = false;
          }
          break;
        }
        if (is_checked) { return;}
      }
      else {
        break;
      }
    }//end step
  } //end direction
  //!check knight positions
}

void newmatch(Newmatch_message message) {
  eos::requireAuth( message.player );
  uint64_t matchid;
  match query;
  bool lastmatch = MainTable::back(query);
  (lastmatch) ? matchid = query.matchid + 1 : matchid = 0 ;
  uint64_t white;
  uint64_t black;
  if (message.side == 0) {
    white = message.player;
    black = message.opponent;
  } else {
    white = message.opponent;
    black = message.player;
  }
  match a;
  a.matchid = matchid;
  a.white = white;
  a.black = black;
  a.status = 0;// 0 started 1 accepted 2 tie 3 game over 4 aborted?
  a.lastmoveside = 1;// 0 white,1 black
  a.moveswhite = 0;
  a.movesblack = 0;
  a.matchstart = now();//unix
  a.check = 10;
  a.kings[0] = 7;
  a.kings[1] = 4;
  a.kings[2] = 0;
  a.kings[3] = 4;
  uint8_t b = 0;
  uint8_t g = 0;
  for (uint8_t i = 0; i < 64; i++) {
    a.board[i] = FRESHBOARD[g][b];
    if (b == 7) {
      g++;
      b = 0;
    } else {
      b++;
    }
  }
  a.graveyard[0] = 10;
  a.graveyard[1] = 10;
  a.graveyard[2] = 10;
  a.graveyard[3] = 10;
  a.graveyard[4] = 10;
  a.graveyard[5] = 10;
  a.graveyard[6] = 10;
  a.graveyard[7] = 10;
  a.graveyard[8] = 10;
  a.graveyard[9] = 10;
  a.graveyard[10] = 10;
  a.graveyard[11] = 10;
  a.graveyard[12] = 10;
  a.graveyard[13] = 10;
  a.graveyard[14] = 10;
  a.graveyard[15] = 10;
  a.lastmove[0] = 10;
  a.lastmove[1] = 10;
  a.lastmove[2] = 10;
  a.lastmove[3] = 10;
  a.lastmove[4] = 10;
  a.castling[0] = 0;
  a.castling[1] = 0;
  a.castling[2] = 0;
  a.castling[3] = 0;
  bool res =  MainTable::store(a);
  if (res == true) {
    eos::print( "Created new match", "\n" );
    //ask player2
  } else {
    eos::print( "Could not create new match", "\n" );
    //why?
  }
}

void castling(Castling_message message) {
  eos::requireAuth( message.player );
  match query;
  query.matchid = message.matchid;
  bool matchexist = MainTable::get(query);
  //assert status if match was accpected needs method too
  assert( matchexist, "Match not found!" );
  assert( query.white == message.player || query.black == message.player, "Player not found!" );
  uint8_t playerside;
  (query.white == message.player) ?  playerside = 0 : playerside = 1;
  assert( playerside != query.lastmoveside, "It's not your turn!" );
  uint8_t board[8][8];
  db_array_to_board(board, query.board);
  bool is_checked = false;
  if (message.type) { //if short castling
    if (!playerside) { //if white
      assert(!query.castling[1] && !board[7][5] && !board[7][6], "Short castling is not possible either because it has already been done or the rook or king have been moved or there are other pieces in between");
      detect_check(board, 7, 4, is_checked);
      assert( !is_checked, "Cannot castle while checked" );
      detect_check(board, 7, 5, is_checked);
      detect_check(board, 7, 6, is_checked);
      assert(!is_checked, "Short castling is not possible because king would be checked while moving to the end position");
      detect_check(board, 7, 7, is_checked);
      assert(!is_checked, "Short castling is not possible because king would be checked at the end of the move");
      board[7][4] = 0;
      board[7][5] = 14;
      board[7][6] = 11;
      board[7][7] = 0;
      query.castling[0] = 1;
      query.castling[1] = 2;
      query.kings[1] = 6;
    }
    else {
      assert(!query.castling[3] && !board[0][5] && !board[0][6], "Short castling is not possible either because it has already been done or the rook or king have been moved or there are other pieces in between");
      detect_check(board, 0, 4, is_checked);
      assert( !is_checked, "Cannot castle while checked" );
      detect_check(board, 0, 5, is_checked);
      detect_check(board, 0, 6, is_checked);
      assert(!is_checked, "Short castling is not possible because king would be checked while moving to the end position");
      detect_check(board, 0, 7, is_checked);
      assert(!is_checked, "Short castling is not possible because king would be checked at the end of the move");
      board[0][4] = 0;
      board[0][5] = 34;
      board[0][6] = 31;
      board[0][7] = 0;
      query.castling[2] = 1;
      query.castling[3] = 2;
      query.kings[3] = 6;
    }
  }
  else {
    if (!playerside) {
      assert(!query.castling[0] && !board[7][1] && !board[7][2] && !board[7][3], "Long castling is not possible either because it has already been done or the rook or king have been moved or there are other pieces in between");
      detect_check(board, 7, 4, is_checked);
      assert( !is_checked, "Cannot castle while checked" );
      detect_check(board, 7, 1, is_checked);
      detect_check(board, 7, 2, is_checked);
      detect_check(board, 7, 3, is_checked);
      assert(!is_checked, "Short castling is not possible because king would be checked while moving to the end position");
      detect_check(board, 7, 0, is_checked);
      assert(!is_checked, "Short castling is not possible because king would be checked at the end of the move");
      board[7][4] = 0;
      board[7][3] = 13;
      board[7][2] = 11;
      board[7][1] = 0;
      board[7][0] = 0;
      query.castling[0] = 2;
      query.castling[1] = 1;
      query.kings[1] = 2;
    }
    else {
      assert(!query.castling[2] && !board[0][1] && !board[0][2] && !board[0][3], "Long castling is not possible either because it has already been done or the rook or king have been moved or there are other pieces in between");
      detect_check(board, 0, 4, is_checked);
      assert( !is_checked, "Cannot castle while checked" );
      detect_check(board, 0, 1, is_checked);
      detect_check(board, 0, 2, is_checked);
      detect_check(board, 0, 3, is_checked);
      assert(!is_checked, "Short castling is not possible because king would be checked while moving to the end position");
      detect_check(board, 0, 0, is_checked);
      assert(!is_checked, "Short castling is not possible because king would be checked at the end of the move");
      board[0][4] = 0;
      board[0][3] = 33;
      board[0][2] = 31;
      board[0][1] = 0;
      board[0][0] = 0;
      query.castling[2] = 2;
      query.castling[3] = 1;
      query.kings[3] = 2;
    }
  }
  query.lastmoveside = playerside;
  uint8_t b = 0;
  uint8_t g = 0;
  for (uint8_t i = 0; i < 64; i++) {
    query.board[i] = board[g][b];
    if (b == 7) {
      g++;
      b = 0;
    } else {
      b++;
    }
  }
  bool res =  MainTable::update(query);
  if (res == true) {
    eos::print( "saved castling move", "\n" );
    //ask player2
  } else {
    eos::print( "couldnt save castling move", "\n" );
    //why?
  }
}

void movepiece(Move_message message) {
  eos::requireAuth( message.player );
  match query;
  query.matchid = message.matchid;
  bool matchexist = MainTable::get(query);
  //assert status if match was accpected needs method too
  assert( matchexist, "Match not found!" );
  assert( query.white == message.player || query.black == message.player, "Player not found!" );
  uint8_t playerside;
  (query.white == message.player) ?  playerside = 0 : playerside = 1;
  assert( playerside != query.lastmoveside, "It's not your turn!" );
  uint8_t board[8][8];
  db_array_to_board(board, query.board);
  uint8_t piece = board[message.steps[0]][message.steps[1]];
  assert( piece != 0 , "There is no piece on this position" );
  if (playerside == 0) {
    assert( piece_side(piece) == 0, "Piece belongs to opponent" );
  } else {
    assert( piece_side(piece) == 1, "Piece belongs to opponent" );
  }
  bool king = false;
  //piece side white 0 black 1,forward only,diagonal steps,vertical steps,horizontal steps,skip pieces(horse)
  uint8_t  piece_config[6];
  uint8_t piece_type = get_piece_type(piece);
  switch (piece_type) {
  case 0:
    if (piece == 11) {
      query.castling[0] = 2; query.castling[1] = 2;
    }
    else {
      query.castling[2] = 2; query.castling[3] = 2;
    }
    add_piece_config(piece_config, {piece, 0, 1, 1, 1, 0});
    king = true;
    break;
  case 1:
    add_piece_config(piece_config, {piece, 0, 7, 7, 7, 0});
    break;
  case 2:
    if (piece == 13) {
      query.castling[0] = 2;
    }
    else if (piece == 14) {
      query.castling[1] = 2;
    }
    else if (piece == 33) {
      query.castling[3] = 2;
    }
    else if (piece == 34) {
      query.castling[2] = 2;
    }
    add_piece_config(piece_config, {piece, 0, 0, 7, 7, 0});
    break;
  case 3:
    add_piece_config(piece_config, {piece, 0, 7, 0, 0, 0});
    break;
  case 4:
    add_piece_config(piece_config, {piece, 0, 0, 2, 2, 1});
    break;
  case 5:
    // ALSO special bois !!!!!!!!!!!!!!!impl pawn promotion
    add_piece_config(piece_config, {piece, 1, 1, 1, 0, 0});
    break;
  }

  uint8_t horizontal_steps = 0;
  uint8_t vertical_steps = 0;
  uint8_t diagonal_steps = 0;
  uint8_t total_steps = 0;
  uint8_t last_position[2] = {message.steps[0], message.steps[1]};
  bool is_checked = false;
  uint8_t en_passant = 0;
  for (uint8_t x = 2; x < message.steps_len / 2; x += 2) {
    if (message.steps[x] > 7) {break;} //as for now 10 means end of steps
    uint8_t occ_piece = board[message.steps[x]][message.steps[x + 1]];
    assert( (int)message.steps[x] - (int)last_position[0] > -2 && (int)message.steps[x] - (int)last_position[0] < 2, "Vertical step is too far" );
    assert( (int)message.steps[x + 1] - (int)last_position[1] > -2 && (int)message.steps[x + 1] - (int)last_position[1] < 2, "Horinzontal step is too far" );
    if (message.steps[x] != last_position[0] && message.steps[x + 1] != last_position[1]) { //diagonal step
      if (piece_config[1]) {
        if (!playerside) { //if white
          assert((int)message.steps[x] - (int)last_position[0] < 0, "Pawn can only move in opponents direction");
          if (piece_side(occ_piece) == 100) { //if empty
            //en passant
            assert(query.lastmove[1] == 1 && query.lastmove[3] == 3 && query.lastmove[0] > 38 && query.lastmove[0] < 47 && message.steps[x] == 2 && message.steps[x + 1] == query.lastmove[4], "Pawn can only move diagonally if an opponent piece is occupying the target field or in an en passant situation");
            en_passant = board[query.lastmove[3]][query.lastmove[4]];
          }
          else {
            assert(piece_side(occ_piece) , "Pawn can only move diagonally if an opponent piece is occupying the target field or in an en passant situation");
          }
        }
        else {
          assert((int)message.steps[x] - (int)last_position[0] > 0, "Pawn can only move in opponents direction");
          if (piece_side(occ_piece) == 100) { //if empty
            //en passant
            assert(query.lastmove[1] == 6 && query.lastmove[3] == 4 && query.lastmove[0] > 18 && query.lastmove[0] < 27 && message.steps[x] == 5 && message.steps[x + 1] == query.lastmove[4], "Pawn can only move diagonally if an opponent piece is occupying the target field or in an en passant situation");
            en_passant = board[query.lastmove[3]][query.lastmove[4]];
          }
          else {
            assert(!piece_side(occ_piece), "Pawn can only move diagonally if an opponent piece is occupying the target field or in an en passant situation");
          }
        }
        piece_config[3] = 0;//remove vertical step
      }
      diagonal_steps++; total_steps++;
      assert(piece_config[2] >= diagonal_steps, "Piece cannot move diagonally or has no more diagonal steps left");
    }
    else if (message.steps[x] != last_position[0] && !diagonal_steps && message.steps[x + 1] == last_position[1]) { //vertical step
      if (piece_config[5] && !vertical_steps && horizontal_steps) {
        assert(horizontal_steps == 2, "Knight must have done two horizontal steps first");
      }
      if (piece_config[1]) {
        assert( piece_side(occ_piece) == 100, "Pawn can only move horinzontally if target field is not occupied");
        if (!playerside) { //if white
          assert((int)message.steps[x] - (int)last_position[0] < 0, "Pawn can only move in opponents direction");
          if (last_position[0] == 6) {
            piece_config[3] = 2; //increase to two steps
          }
        }
        else {
          assert((int)message.steps[x] - (int)last_position[0] > 0, "Pawn can only move in opponents direction");
          if (last_position[0] == 1) {
            piece_config[3] = 2; //increase to two steps
          }
        }
        piece_config[2] = 0;//remove diagonal step
      }
      vertical_steps++; total_steps++;
      assert(piece_config[3] >= vertical_steps, "Piece cannot move vertically or has no more vertical steps left");
    }
    else if (message.steps[x + 1] != last_position[1] && !diagonal_steps && message.steps[x] == last_position[0]) { //horizontal step
      if (piece_config[5] && !horizontal_steps && vertical_steps) {
        assert(vertical_steps == 2 , "Knight must have done two vertical steps first");
      }
      horizontal_steps++; total_steps++;
      assert(piece_config[4] >= horizontal_steps, "Piece cannot move horinzontally or has no more horizontal steps left");
    }
    if (piece_config[5] && total_steps < 3) {//knight is not fnished
      //update last position
      last_position[0] = message.steps[x];
      last_position[1] = message.steps[x + 1];
      continue;
    }
    if (piece_side(occ_piece) != 100) {//next piece is not empty
      if (piece_side(occ_piece) != playerside) {
        for (uint8_t uu = 0; uu < 16; uu++) { //add to graveyard
          if (query.graveyard[uu] == 0) {
            query.graveyard[uu] = occ_piece;
            board[message.steps[x]][message.steps[x + 1]] = 0; //remove from board
            break;// end of graveyard loop
          }
        }
        //change position on board
        board[last_position[0]][last_position[1]] = 0;
        board[message.steps[x]][message.steps[x + 1]] = piece;
        break;//end of steps loop
      }
      else {
        assert(false, "Piece cannot move through your own pieces.");
      }
    }
    else {
      if (en_passant) {
        for (uint8_t uu = 0; uu < 16; uu++) { //add to graveyard
          if (query.graveyard[uu] == 0) {
            query.graveyard[uu] = en_passant;
            board[query.lastmove[3]][query.lastmove[4]] = 0; //remove from board
            break;// end of graveyard loop
          }
        }
      }
    }
    //update last position
    last_position[0] = message.steps[x];
    last_position[1] = message.steps[x + 1];
    //break;
  }
  board[last_position[0]][last_position[1]] = piece;
  board[message.steps[0]][message.steps[1]] = 0;
  query.lastmove[0] = piece;
  query.lastmove[1] = message.steps[0];
  query.lastmove[2] = message.steps[1];
  query.lastmove[3] = last_position[0];
  query.lastmove[4] = last_position[1];
  query.lastmove[0] = piece;
  query.lastmove[1] = message.steps[0];
  query.lastmove[2] = message.steps[1];
  query.lastmove[3] = last_position[0];
  query.lastmove[4] = last_position[1];
  if (!playerside) {
    if (king) {
      query.kings[0] = last_position[0];
      query.kings[1] = last_position[1];
    }
    detect_check(board, query.kings[0], query.kings[1], is_checked);
  }
  else {
    if (king) {
      query.kings[2] = last_position[0];
      query.kings[3] = last_position[1];
    }
    detect_check(board, query.kings[2], query.kings[3], is_checked);
  }
  assert(!is_checked, "You cannot end your move if your king is in check");
  query.lastmoveside = playerside;
  uint8_t b = 0;
  uint8_t g = 0;
  for (uint8_t i = 0; i < 64; i++) {
    query.board[i] = board[g][b];
    printi((int)query.board[i]);
    if (b == 7) {
      g++;
      b = 0;
    } else {
      b++;
    }
  }
  printi(query.matchid);
  eos::print(" matchid", "\n");
  printn(query.white);
  eos::print(" white", "\n");
  printn(query.black);
  eos::print(" black", "\n");
  printi(query.status);
  eos::print(" status", "\n");
  printi(query.lastmoveside);
  eos::print(" lastmoveside", "\n");
  printi(query.moveswhite);
  eos::print(" moveswhite", "\n");
  printi(query.movesblack);
  eos::print(" movesblack", "\n");
  printi(query.matchstart);
  eos::print(" matchstart", "\n");
  printi(query.lastmove[0]);
  printi(query.lastmove[1]);
  printi(query.lastmove[2]);
  printi(query.lastmove[3]);
  printi(query.lastmove[4]);
  eos::print(" lastmove", "\n");
  printi(query.check);
  eos::print(" check", "\n");
  printi(query.kings[0]);
  printi(query.kings[1]);
  printi(query.kings[2]);
  printi(query.kings[3]);
  eos::print(" kings", "\n");
  //IMPORTANT impl check method and after that castling
  bool res =  MainTable::update(query);
  if (res == true) {
    eos::print( "saved move", "\n" );
    //ask player2
  } else {
    eos::print( "couldnt save move", "\n" );
    //why?
  }
}

extern "C" {

  void init()  {

  }
  /// The apply method implements the dispatch of events to this contract
  void apply( uint64_t code, uint64_t action ) {
    if ( code == N(chess) ) {
      switch ( action ) {
      case N( newmatch ):
        eos::print("action newmatch ", "\n");
        newmatch( eos::currentMessage<Newmatch_message>() );
        break;
      case N( movepiece ):
        eos::print("action movepiece ", "\n");
        movepiece( eos::currentMessage<Move_message>() );
        break;
      case N( castling ):
        eos::print("action castling ", "\n");
        castling( eos::currentMessage<Castling_message>() );
        break;
      default :
        assert( false, "unknown action" );
      }
    }
  }
} // extern "C"

