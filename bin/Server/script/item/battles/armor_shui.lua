Include("\\script\\header\\forbidmap.lua")

function main(nItemIdx)
	local W,X,Y = GetWorldPos();
	local nMapId = W;	
	if ( checkSJMaps(nMapId) ~= 1 ) then
		Msg2Player("L�c n�y b�n kh�ng th� s� d�ng v�t ph�m n�y");
		return 1;
	end
	RemoveItem(nItemIdx,1);
	level = random(1,7);
	AddSkillState(476,level,1,1080)
	Msg2Player("B�n nh�n ���c<color=cyan> t�ng ph�ng h�a trong 1 ph�t "..(level*10).."%")	
end

