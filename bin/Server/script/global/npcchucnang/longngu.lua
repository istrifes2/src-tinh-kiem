Include("\\script\\header\\taskid.lua");
Include("\\script\\lib\\worldlibrary.lua");

function main()
	if(SERVER_MODE == 1) then
	Talk(1,"",SERVER_NOTICE)
	return end
	Say("LINK:<npc>",0,10194,3,"Ta ��n l�m nhi�m v� Ch�nh tuy�n c�p 20 tr� l�n/nhiemvuchinh","Ta ��n xem gi�i thi�u v� nhi�m v� /xemgioithieu","Sau n�y h�y n�i/no")
end;

function xemgioithieu()
	Say("LINK:<npc>",0,10243,1,"K�t th�c ��i tho�i/no")
end;

function nhiemvuchinh()
	local nTaskValue = GetTask(TASK_NVST);
	local nChinh = GetNumber(2,nTaskValue,2);
	local nTrung = GetNumber(2,nTaskValue,3);
	local nTa = GetNumber(2,nTaskValue,4);
	if(nChinh > 19 and nTrung > 20 and nTa > 29) then
	Talk(1,"","Sau n�y ng��i ph�i t� b�o tr�ng!");
	return end;
	if(nChinh == 19) then
	Talk(1,"donechinh",10340);
	return end
	if(nTrung == 20) then
	Talk(1,"donetrung",14746);
	return end
	if(nTa == 29 and GetItemCount(0,4,463)>0) then
	Talk(1,"doneta","C� ch�t qu� t�ng cho ng��i!");
	return end
	if(GetLevel() < 20) then
	Talk(1,"",10344);
	return end;
	if(GetFactionNo() < 0) then
	Talk(1,"",12636);
	return end;
	Say(10345,2,"Ta mu�n nh�n nhi�m v� c�p 20 tr� l�n/selnv","Ta ch� ��n ch�i/no")
end;

function selnv()
	Say(10346,4,"Ch�nh ph�i/nhannv","Trung l�p/nhannv","T� ph�i/nhannv","�� ta suy ngh� l�i/no")
end;

function nhannv(sel)
	local nSel = sel + 2;
	local nTaskValue = GetTask(TASK_NVST);
	if(GetNumber(2,nTaskValue,nSel) > 0) then
		Talk(1,"",10369);
	return end;
	if(CheckRoom(1,1) == 0) then
		Talk(1,"",12265);
	return end;
	SetTask(TASK_NVST, SetNumber(2,nTaskValue,nSel,1));
	local nNewItem = AddItemID(AddItem(0,6,130,0,0,5,0));
	SetParamItem(nNewItem,sel);
	Msg2Player("Nh�n ���c m�t t�i th� c�a Long Ng�.")
end;

function donechinh()
	SetTask(TASK_NVST, SetNumber(2,GetTask(TASK_NVST),2,20));
	AddOwnExp(300000);
	AddItemID(AddItem(0,4,random(490,501),0,0,5,0,0));
	if(random(1,2) == 1) then
		AddItemID(AddItem(2,0,RANDOM(167,175),0,0,0,10));
	else
		if(GetSex() == 0) then
		AddItemID(AddItem(2,0,RANDOM(167,175),0,0,0,5));
		else
		AddItemID(AddItem(2,0,RANDOM(167,175),0,0,0,5));
		end
	end
	Msg2Player("B�n B�n nh�n ���c m�t m�n b�o kh�.")
	Msg2Player("B�n �� ho�n th�nh nhi�m v� M�c S�u giao. Long Ng� v� c�ng h�i l�ng, nh�ng b��c ���ng giang h� ch� m�i b�t ��u.")
end;

function donetrung()
	SetTask(TASK_NVST, SetNumber(2,GetTask(TASK_NVST),3,21));
	AddOwnExp(300000);
	AddItemID(AddItem(0,4,random(490,501),0,0,5,0,0));
	if(random(1,2) == 1) then
		AddItemID(AddItem(2,0,RANDOM(167,175),0,0,0,10));
	else
		if(GetSex() == 0) then
		AddItemID(AddItem(2,0,RANDOM(167,175),0,0,0,5));
		else
		AddItemID(AddItem(2,0,RANDOM(167,175),0,0,0,5));
		end
	end
	Msg2Player("B�n B�n nh�n ���c m�t m�n b�o kh�.")
	Msg2Player("B�n �� ho�n th�nh nhi�m v� s�t th� Ph� Nam B�ng giao. Tr�n giang h� b�n �� c� ch�t danh ti�ng, ��y c�ng kh�ng ph�i l� chuy�n t�t, trong th�nh c� nhi�u tai m�t theo d�i.")
end;

function doneta()
	if(DelItem(1,0,4,463)==1) then
	SetTask(TASK_NVST, SetNumber(2,GetTask(TASK_NVST),4,30));
	AddOwnExp(300000);
	AddItemID(AddItem(0,4,random(490,501),0,0,5,0,0));
	if(random(1,2) == 1) then
		AddItemID(AddItem(2,0,RANDOM(167,175),0,0,0,10));
	else
		if(GetSex() == 0) then
		AddItemID(AddItem(2,0,RANDOM(167,175),0,0,0,5));
		else
		AddItemID(AddItem(2,0,RANDOM(167,175),0,0,0,5));
		end
	end
	Msg2Player("B�n B�n nh�n ���c m�t m�n b�o kh�.")
	Msg2Player("B�n �� ho�n th�nh nhi�m v� ch� tuy�n T� ph�i. Sau n�y c� l� c� nhi�u tr�n chi�n �ang ch� b�n, m�t k� m�u l�nh.")
	end
end;

function no()
end;