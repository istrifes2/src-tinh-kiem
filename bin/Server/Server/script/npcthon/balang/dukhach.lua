
function main()
	if(GetItemCount(0,4,183) > 0) then
	Say(11282,2,
		"B�n/ban",
		"Kh�ng b�n/no")
	else
	Talk(1,"",random(11284,11286))
	end
end;

function ban()
	if(GetItemCount(0,4,183) > 0) then
	DelItem(1,0,4,183)
	Earn(1500)
	Talk(1,"",11287)
	Msg2Player("��a b�c th� ph�p cho ng��i du kh�ch l�y ���c 1500 l��ng")
	end
end;

function no()
	Talk(1,"",11288)
end;
