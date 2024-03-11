let list_of_player_requests = "List of game requests. Click to accept the game request.<br><br>";
let game_requests_sent_list = "";

const webSocket = new WebSocket("ws://localhost:8000");

webSocket.onopen = function(event) {
    console.log("\nPlayer successfully connected to the Tic-Tac-Toe Server.\n\n");
};
webSocket.onclose = function (event) {
    console.log("\nPlayer disconnected from the Tic-Tac-Toe Server.\n\n");
};
webSocket.onerror = function (event) {
    console.error("WebSocket error! ", event);
};
webSocket.onmessage = function(event) {
	console.log("Received message from the Server: ", event.data);
	if (event.data.includes("Player"))
	{
		document.getElementById("game_request_section").style.display = "initial";
		document.getElementById("player_id").innerHTML = event.data;
	}
	if (event.data.includes("active_players"))
	{
		let active_players_list = event.data.split(" ");
		let list_of_players = "List of active players. Click to send game request to the player.<br><br>"; 
		for(let i = 1; i < active_players_list.length-1; i++)
		{
			let opp_id = active_players_list[i];
			list_of_players += "<button id='player_id_button' onclick='send_game_request(" + opp_id + ")'>" + opp_id + "</button><br>";
		}
		document.getElementById("active_players_list").style.display = "initial";
		document.getElementById("active_players_list").innerHTML = list_of_players;
	}
	if (event.data.includes("no_active_players"))
	{
		document.getElementById("active_players_list").style.display = "initial";
		document.getElementById("active_players_list").innerHTML = "Currently there are no active players.<br>";
	}
	if (event.data.includes("Game request from playerID"))
	{
		let game_request_received = event.data.split(" ");
		let player_id = game_request_received[4];
		list_of_player_requests += "<button id='player_id_button' onclick='accept_game_request(" + player_id + ")'>" + player_id + "</button><br>";
		document.getElementById("game_requests_received_list").style.display = "initial";
		document.getElementById("game_requests_received_list").innerHTML = list_of_player_requests;
	}
	if (event.data.includes("updated_players_list"))
	{
		let game_requests_received_list = event.data.split(" ");
		let updated_list_of_player_requests = "List of game requests. Click to accept the game request.<br><br>";
		for(let i = 1; i < game_requests_received_list.length-1; i++)
		{
			let player_id = game_requests_received_list[i];
			updated_list_of_player_requests += "<button id='player_id_button' onclick='accept_game_request(" + player_id + ")'>" + player_id + "</button><br>";
		}
		document.getElementById("game_requests_received_list").style.display = "initial";
		document.getElementById("game_requests_received_list").innerHTML = updated_list_of_player_requests;
	}
	if (event.data.includes("no_player_requests"))
	{
		document.getElementById("game_requests_received_list").style.display = "initial";
		document.getElementById("game_requests_received_list").innerHTML = "Currently there are no requests from other players.";
	}
	if (event.data.includes("Game request accepted"))
	{
		document.getElementById("game_request_section").style.display = "none";
		document.getElementById("board_section").style.display = "initial";
	}
	if (event.data.includes("vs"))
	{
		let battle_message = event.data.split(" ");
		document.getElementById("battle_message").innerHTML = "Player " + battle_message[0] + " vs " + "Player " +  battle_message[2];
	}
	if (event.data.includes("Symbol"))
		document.getElementById("player_symbol").innerHTML = "You are " + event.data[7];
	if (event.data.includes("Your turn"))
	{
		document.getElementById("board").style.pointerEvents = "auto";
		document.getElementById("game_status").innerHTML = event.data;
	}
	if (event.data.includes("Opponent's turn"))
	{
		document.getElementById("board").style.pointerEvents = "none";
		document.getElementById("game_status").innerHTML = event.data;
	}
	if (event.data.includes(","))
	{
		let [row, cell, symbol] = event.data.split(",");
		var x = document.getElementById("board").rows[row].cells;
		x[cell].innerHTML = symbol;
		x[cell].style.backgroundColor = "white";
		x[cell].style.color = "black";		
	}
	if (event.data.includes("-"))
	{
		let winning_cells_string_array = event.data.split("-");
		let winning_cells_int_array = winning_cells_string_array.map(Number);
		for(let i=0; i<winning_cells_int_array.length; i++)	
		{
			let r = parseInt(winning_cells_int_array[i]/3);
			let c = parseInt(winning_cells_int_array[i]%3);
			var x = document.getElementById("board").rows[r].cells;
			x[c].style.backgroundColor = "OrangeRed";
			x[c].style.color = "black";
		}
	}
	if (event.data.includes("won"))
	{
		document.getElementById("board").style.pointerEvents = "none";
		document.getElementById("game_status").innerHTML = event.data;
		document.getElementById("game_status").style.color = "OrangeRed";
	}
	if (event.data.includes("lost"))
	{
		document.getElementById("board").style.pointerEvents = "none";
		document.getElementById("game_status").innerHTML = event.data;
		document.getElementById("game_status").style.color = "OrangeRed";
	}
	if (event.data.includes("draw"))
	{
		document.getElementById("board").style.pointerEvents = "none";
		document.getElementById("game_status").innerHTML = event.data;
		document.getElementById("game_status").style.color = "OrangeRed";
	}
};

function send_game_request(opp_id)
{
	document.getElementById("game_requests_sent_list").style.display = "initial";
	if(!game_requests_sent_list.includes(opp_id))
	{
		webSocket.send("game_request " + opp_id);
		game_requests_sent_list += "Game request sent to player " + opp_id + "<br>";
		document.getElementById("game_requests_sent_list").innerHTML = game_requests_sent_list;
	}
}

function accept_game_request(opp_id)
{
	webSocket.send("accept_request " + opp_id);
}

function move(row, cell)
{
	webSocket.send(row + " " + cell + " move");
}
