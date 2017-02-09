$(document).ready(function(){
	//alert('Working!');
	var size, arrsize, arr;
	var score = 0;

	// Create the array and generate the first cell
	(function arrayStart(){
		size = 4;
		arrsize = size * size;
		arr = new Array(arrsize).fill(0);
	})();

	// Update the js array with the current status of the board
	var updateStatus = function(){
		var currcell;
		for (var i = 0; i < size; i++) {
			for (var j = 0; j < size; j++) {
				// There should only be one cell if there is one
				currcell = $('.overlay.row-' + i + '.col-' + j);
				//if (currcell.length) console.log('currcell: ', currcell.first().html());
				arr[size*i+j] = currcell.length > 0 ? parseInt(currcell.first().html()) : 0;
			}
		}
	};

	// Return the number of open spaces
	var getOpen = function () {
		updateStatus();
		var count = 0;
		for(var i = 0; i < arrsize; i++) { if (arr[i] == 0) {count++;}  }
		return count;
	}

	// Check to see if we are done
	var checkDone = function () {
		var full = 0;
		for (var i = 0; i < 16; i++) {
			if (arr[i]) full++;
		}

		if (full = 16) {
			// Check left
			for (var i = 0; i < 4; i++) {
				last = 0;
				for (var j = 0; j < 4; j++) {
					if (last == arr[size*i+j]) return false;
					last = arr[size*i+j];
				}
			}

			// Check right
			for (var i = 0; i < 4; i++) {
				last = 0;
				for (var j = 3; j >= 0; j--) {
					if (last == arr[size*i+j]) return false;
					last = arr[size*i+j];
				}
			}

			// Check up
			for (var j = 0; j < 4; j++) {
				last = 0;
				for (var i = 0; i < 4; i++) {
					if (last == arr[size*i+j]) return false;
					last = arr[size*i+j];
				}
			}

			// Check down
			for (var j = 0; j < 4; j++) {
				last = 0;
				for (var i = 3; i >= 0; i--) {
					if (last == arr[size*i+j]) return false;
					last = arr[size*i+j];
				}
			}

			alert('No more moves left!  Thnx for playin!  Final Score: ' + score);
		}
	}

	// Generate a new div in one of the open spaces
	var genNew = function () {
		var totalOpen = getOpen();
		var selectRand = Math.floor(Math.random() * totalOpen);
		var count = 0;
		var prodRand = Math.floor(Math.random() * 10);
		for(var i = 0; i < arrsize; i++) {
			if (arr[i] == 0) {
				// This is the random location to place the new div
				if (count == selectRand) {
					arr[i] = prodRand == 0 ? 4 : 2;
					$('.overallFrame').append('<div class="color-' + arr[i] + ' cell overlay row-' + Math.floor(i/size) + ' col-' + (i%size) + '">' + arr[i] + '</div>');
					return;
				}
				// Otherwise increment the count
				else {
					count++;
				}
			}
		}
		//alert('Congrats...you can now watch the world burn!');
	}

	// If the user presses the left key then move all of the boxes to the left
	$(document).keydown(function (e) {
		updateStatus();
		//console.log('current array: ', arr);
		//console.log('Inside left function: ' + e.which);
		// Open: num spots that need box needs to be moved; Last: the number in the previous box
		var open = 0;
		var last = 0;
		var count = 0;
		var key = e.which;
		switch (key) {
			// When the user presses enter make a new box
			case 13: genNew(); break;

			// Left arrow move all the boxes to the left
			case 37:
				for (var i = 0; i < 4; i++) {
					open = 0;
					last = 0;
					for (var j = 0; j < 4; j++) {
						//console.log('log for each cell: ' + i + ' ' + j + ' ' + arr[size*i+j]);
						if (!arr[size*i+j]) {
							open++;
						}
						else {
							if (last == arr[size*i+j]) {
								open++;
								score += 2 * arr[size*i+j];
								$('#scoreBoard').html('Score: ' + score);
								$('.overlay.row-' + i + '.col-' + j).removeClass('color-' + last).addClass('color-' + (2*last)).addClass('active').off('transitionend webkitTransitionEnd oTransitionEnd otransitionend MSTransitionEnd').on('transitionend webkitTransitionEnd oTransitionEnd otransitionend MSTransitionEnd', { finval: 2 * arr[size*i+j], row: i, col: j-open} , function (e) {
									//console.log($(this).html());
									$(this).html(e.data.finval);
									$('.overlay.row-' + e.data.row + '.col-' + e.data.col + ':not(.active)').remove();
									$(this).off('transitionend webkitTransitionEnd oTransitionEnd otransitionend MSTransitionEnd').on('transitionend webkitTransitionEnd oTransitionEnd otransitionend MSTransitionEnd', function () {
										//console.log('here');
										$(this).removeClass('pulse');
									});
									$(this).removeClass('active').addClass('pulse');
								});
							}
							else {
								arr[size*i+(j-open)] = arr[size*i+j];
							}
							if (open > 0) count++;
							$('.overlay.row-' + i + '.col-' + j).removeClass('col-' + j).addClass('col-' + (j-open));
							//$('.overlay.row-' + i + '.col-' + (j-open) + ':not(.active)').remove();
							//$('.overlay.active.row-' + i + '.col-' + (j-open)).removeClass('active');
							last = last == arr[size*i+j] ? 0 : arr[size*i+j];
						}
					}
				}
				if (count > 0) setTimeout(function () {genNew(); checkDone();}, 200);
				break;

			// Right arrow move all the boxes to the right
			case 39:
				for (var i = 0; i < 4; i++) {
					open = 0;
					last = 0;
					for (var j = 3; j >= 0; j--) {
						//console.log('log for each cell: ' + i + ' ' + j + ' ' + arr[size*i+j]);
						if (!arr[size*i+j]) {
							open++;
						}
						else {
							if (last == arr[size*i+j]) {
								open++;
								score += 2 * arr[size*i+j];
								$('#scoreBoard').html('Score: ' + score);
								$('.overlay.row-' + i + '.col-' + j).removeClass('color-' + last).addClass('color-' + (2*last)).addClass('active').off('transitionend webkitTransitionEnd oTransitionEnd otransitionend MSTransitionEnd').on('transitionend webkitTransitionEnd oTransitionEnd otransitionend MSTransitionEnd', { finval: 2 * arr[size*i+j], row: i, col: j+open} , function (e) {
									//console.log($(this).html());
									$(this).html(e.data.finval);
									$('.overlay.row-' + e.data.row + '.col-' + e.data.col + ':not(.active)').remove();
									$(this).off('transitionend webkitTransitionEnd oTransitionEnd otransitionend MSTransitionEnd').on('transitionend webkitTransitionEnd oTransitionEnd otransitionend MSTransitionEnd', function () {
										//console.log('here');
										$(this).removeClass('pulse');
									});
									$(this).removeClass('active').addClass('pulse');
								});
							}
							else {
								arr[size*i+(j+open)] = arr[size*i+j];
							}
							if (open > 0) count++;
							$('.overlay.row-' + i + '.col-' + j).removeClass('col-' + j).addClass('col-' + (j+open));
							//$('.overlay.row-' + i + '.col-' + (j+open) + ':not(.active)').remove();
							//$('.overlay.active.row-' + i + '.col-' + (j+open)).removeClass('active');
							last = last == arr[size*i+j] ? 0 : arr[size*i+j];
						}
					}
				}
				if (count > 0) setTimeout(function () {genNew(); checkDone();}, 200);
				break;

			// Up arrow move all the boxes to the up
			case 38:
				for (var j = 0; j < 4; j++) {
					open = 0;
					last = 0;
					for (var i = 0; i < 4; i++) {
						//console.log('log for each cell: ' + i + ' ' + j + ' ' + arr[size*i+j]);
						if (!arr[size*i+j]) {
							open++;
						}
						else {
							if (last == arr[size*i+j]) {
								open++;
								score += 2 * arr[size*i+j];
								$('#scoreBoard').html('Score: ' + score);
								$('.overlay.row-' + i + '.col-' + j).removeClass('color-' + last).addClass('color-' + (2*last)).addClass('active').off('transitionend webkitTransitionEnd oTransitionEnd otransitionend MSTransitionEnd').on('transitionend webkitTransitionEnd oTransitionEnd otransitionend MSTransitionEnd', { finval: 2 * arr[size*i+j], row: i-open, col: j} , function (e) {
									//console.log($(this).html());
									$(this).html(e.data.finval);
									$('.overlay.row-' + e.data.row + '.col-' + e.data.col + ':not(.active)').remove();
									$(this).off('transitionend webkitTransitionEnd oTransitionEnd otransitionend MSTransitionEnd').on('transitionend webkitTransitionEnd oTransitionEnd otransitionend MSTransitionEnd', function () {
										//console.log('here');
										$(this).removeClass('pulse');
									});
									$(this).removeClass('active').addClass('pulse');
								});
							}
							else {
								arr[size*(i-open)+j] = arr[size*i+j];
							}
							if (open > 0) count++;
							$('.overlay.row-' + i + '.col-' + j).removeClass('row-' + i).addClass('row-' + (i-open));
							last = last == arr[size*i+j] ? 0 : arr[size*i+j];
						}
					}
				}
				if (count > 0) setTimeout(function () {genNew(); checkDone();}, 200);
				break;

			// Down arrow move all the boxes to the down
			case 40:
				for (var j = 0; j < 4; j++) {
					open = 0;
					last = 0;
					for (var i = 3; i >= 0; i--) {
						//console.log('log for each cell: ' + i + ' ' + j + ' ' + arr[size*i+j]);
						if (!arr[size*i+j]) {
							open++;
						}
						else {
							if (last == arr[size*i+j]) {
								open++;
								score += 2 * arr[size*i+j];
								$('#scoreBoard').html('Score: ' + score);
								$('.overlay.row-' + i + '.col-' + j).removeClass('color-' + last).addClass('color-' + (2*last)).addClass('active').off('transitionend webkitTransitionEnd oTransitionEnd otransitionend MSTransitionEnd').on('transitionend webkitTransitionEnd oTransitionEnd otransitionend MSTransitionEnd', { finval: 2 * arr[size*i+j], row: i+open, col: j} , function (e) {
									//console.log($(this).html());
									$(this).html(e.data.finval);
									$('.overlay.row-' + e.data.row + '.col-' + e.data.col + ':not(.active)').remove();
									$(this).off('transitionend webkitTransitionEnd oTransitionEnd otransitionend MSTransitionEnd').on('transitionend webkitTransitionEnd oTransitionEnd otransitionend MSTransitionEnd', function () {
										//console.log('here');
										$(this).removeClass('pulse');
									});
									$(this).removeClass('active').addClass('pulse');
								});
							}
							else {
								arr[size*(i+open)+j] = arr[size*i+j];
							}
							if (open > 0) count++;
							$('.overlay.row-' + i + '.col-' + j).removeClass('row-' + i).addClass('row-' + (i+open));
							last = last == arr[size*i+j] ? 0 : arr[size*i+j];
						}
					}
				}
				if (count > 0) setTimeout(function () {genNew(); checkDone();}, 200);
				break;

			default:
		}
	});

	// Generate the first cell in the game
	genNew();
	genNew();
});
